target_sources(
    QGCUnitTests
    PRIVATE
        VehicleTranswingFactGroupTest.cc
        VehicleTranswingFactGroupTest.h
)
add_qgc_test(VehicleTranswingFactGroupTest LABELS Unit Vehicle)
