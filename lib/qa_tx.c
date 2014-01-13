/* 
 * Checks that the radio is able to transmit clean sine waves at various
 * frequencies and bitrates.  A spectrum analyzer is required to quantify
 * the results of these tests.
 */

#include <stdio.h>
#include <unistd.h>
#include <termios.h>


#include "whitebox.h"
#include "whitebox_test.h"


char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return (buf);
}

void confirm(char* msg) {
    char c;
    printf("%s? [yn] ", msg);
    fflush(stdout);
    c = getch();
    printf("%c\n", c);
    assert(c == 'y');
}

int test_dds(void *data) {
    whitebox_t wb;
    int fd;
    whitebox_init(&wb);
    assert((fd = whitebox_open(&wb, "/dev/whitebox", O_RDWR, 1e6)) > 0);
    confirm("Is there no carrier at 144.95e6");
    whitebox_tx_dds_enable(&wb, 100e3);
    confirm("Is there a USB tone visible now");
    assert(whitebox_tx(&wb, 144.95e6) == 0);
    confirm("Is there a carrier at 144.95e6");
    whitebox_close(&wb);
    confirm("Is there no more signal");
}

#define N 512
#define SAMPLE_RATE 100e3 
#define DURATION_IN_SECS 3
#define TOTAL_SAMPLES (DURATION_IN_SECS * SAMPLE_RATE)
int test_cic(void * data) {
    float freq = SAMPLE_RATE / 4;
    float sample_rate = SAMPLE_RATE;
    float carrier_freq = 144.95e6;
    uint32_t fcw = freq_to_fcw(freq, sample_rate);
    uint32_t last_phase = 0;
    uint32_t phases[N];
    uint32_t c[N];
    int16_t i, q;
    int n;
    int fd, ret;
    whitebox_t wb;

    whitebox_init(&wb);
    assert((fd = whitebox_open(&wb, "/dev/whitebox", O_RDWR, sample_rate)) > 0);
    assert(whitebox_tx(&wb, carrier_freq) == 0);
    whitebox_debug_to_file(&wb, stdout);

    /*uint32_t phase_sequence = {
        0 << 30,
        1 << 30,
        2 << 30,
        3 << 30
    };*/

    whitebox_tx_dds_enable(&wb, 100e3);

    while (1) {

        for (i = -53; i <= -47; i += 1) {
            /*for (n = 0; n < N; ++n) {
                c[n] =  ((uint32_t)i & 0x0000ffff) << 16;
            }*/

            printf("%d\n", i);
            whitebox_tx_set_correction(&wb, 16, -51);
            for (n = 0; n < TOTAL_SAMPLES; n += N) {
                ret = write(whitebox_fd(&wb), c, sizeof(uint32_t) * N);
                if (ret != sizeof(uint32_t) * N) {
                    printf("U"); fflush(stdout);
                }
            }
        }

    }

    assert(fsync(fd) == 0);

    assert(whitebox_close(&wb) == 0);
}

int main(int argc, char **argv) {
    whitebox_test_t tests[] = {
        WHITEBOX_TEST(test_dds),
        WHITEBOX_TEST(test_cic),
        WHITEBOX_TEST(0),
    };
    return whitebox_test_main(tests, NULL, argc, argv);
}