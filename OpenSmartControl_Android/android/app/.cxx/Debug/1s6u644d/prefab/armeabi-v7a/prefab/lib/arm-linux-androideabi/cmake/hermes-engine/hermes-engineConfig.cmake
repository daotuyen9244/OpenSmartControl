if(NOT TARGET hermes-engine::libhermes)
add_library(hermes-engine::libhermes SHARED IMPORTED)
set_target_properties(hermes-engine::libhermes PROPERTIES
    IMPORTED_LOCATION "/Users/admin/.gradle/caches/8.14.1/transforms/67bc6afafe74fb80a28413c7f5b5c8cc/transformed/hermes-android-0.80.0-debug/prefab/modules/libhermes/libs/android.armeabi-v7a/libhermes.so"
    INTERFACE_INCLUDE_DIRECTORIES "/Users/admin/.gradle/caches/8.14.1/transforms/67bc6afafe74fb80a28413c7f5b5c8cc/transformed/hermes-android-0.80.0-debug/prefab/modules/libhermes/include"
    INTERFACE_LINK_LIBRARIES ""
)
endif()

