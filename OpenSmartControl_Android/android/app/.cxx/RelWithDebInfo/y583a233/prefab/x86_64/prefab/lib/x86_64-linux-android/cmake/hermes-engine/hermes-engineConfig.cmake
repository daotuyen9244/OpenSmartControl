if(NOT TARGET hermes-engine::libhermes)
add_library(hermes-engine::libhermes SHARED IMPORTED)
set_target_properties(hermes-engine::libhermes PROPERTIES
    IMPORTED_LOCATION "/Users/admin/.gradle/caches/8.14.1/transforms/e3847898c9ad159c30703260241a1547/transformed/hermes-android-0.80.0-release/prefab/modules/libhermes/libs/android.x86_64/libhermes.so"
    INTERFACE_INCLUDE_DIRECTORIES "/Users/admin/.gradle/caches/8.14.1/transforms/e3847898c9ad159c30703260241a1547/transformed/hermes-android-0.80.0-release/prefab/modules/libhermes/include"
    INTERFACE_LINK_LIBRARIES ""
)
endif()

