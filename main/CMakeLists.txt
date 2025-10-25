idf_component_register(
    SRCS "hello_s3.c" "cubic_curve.c" "db_utils.c"
    INCLUDE_DIRS "."
    REQUIRES driver led_strip
)
