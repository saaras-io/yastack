proto_library(
    name = "collector_proto",
    srcs = ["collector.proto"],
    deps = [
        "@lightstep_vendored_googleapis//:googleapis_proto",
        "@com_google_protobuf//:timestamp_proto",
    ],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "collector_proto_cc",
    deps = [":collector_proto"],
    visibility = ["//visibility:public"],
)

proto_library(
    name = "lightstep_carrier_proto",
    srcs = ["lightstep_carrier.proto"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "lightstep_carrier_proto_cc",
    deps = [":lightstep_carrier_proto"],
    visibility = ["//visibility:public"],
)
