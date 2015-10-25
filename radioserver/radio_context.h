#ifndef __RADIO_CONTEXT__
#define __RADIO_CONTEXT__

// Add members to radio_context as you wish.
class radio_context {
private:

  radio_context(const radio_context &);
  radio_context &	operator =(const radio_context &);
  client_context * const client;

public:
    radio_context(client_context * c) : client(c) { };
    ~radio_context() {}

    client_context * get_client() { return client; }
};

#endif