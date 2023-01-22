
set(srcs "main.c")
list(APPEND srcs    
src/adxl345.c
src/ble_hid.c
src/ble_db_table.c
                    )

set(inclu_dir  ".")
list(APPEND inclu_dir "src")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${inclu_dir})
                    


target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)