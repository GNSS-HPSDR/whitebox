configure_file(designer.sdc designer.sdc
    COPYONLY
)

configure_file(synthesis.sdc synthesis.sdc
    COPYONLY
)

add_custom_command(
    OUTPUT designer.pdc
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/a2f_som_bsb_ext.py --src-dir=${CMAKE_CURRENT_SOURCE_DIR} --binary-dir=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS a2f_som_bsb_ext.py designer.pdc.tmpl
)

add_custom_target(constraints
    DEPENDS designer.pdc synthesis.sdc designer.sdc
)
