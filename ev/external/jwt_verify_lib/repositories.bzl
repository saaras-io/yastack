def boringssl_repositories(bind=True):
    native.git_repository(
        name = "boringssl",
        commit = "9df0c47bc034d60d73d216cd0e090707b3fbea58",  # same as Envoy
        remote = "https://boringssl.googlesource.com/boringssl",
    )

    if bind:
        native.bind(
            name = "ssl",
            actual = "@boringssl//:ssl",
        )

def googletest_repositories(bind=True):
    native.new_git_repository(
        name = "googletest_git",
        build_file = "googletest.BUILD",
        commit = "43863938377a9ea1399c0596269e0890b5c5515a",
        remote = "https://github.com/google/googletest.git",
    )

    if bind:
        native.bind(
            name = "googletest",
            actual = "@googletest_git//:googletest",
        )

        native.bind(
            name = "googletest_main",
            actual = "@googletest_git//:googletest_main",
        )

        native.bind(
            name = "googletest_prod",
            actual = "@googletest_git//:googletest_prod",
        )

def rapidjson_repositories(bind=True):
    native.new_git_repository(
        name = "com_github_tencent_rapidjson",
        build_file = "rapidjson.BUILD",
        commit = "f54b0e47a08782a6131cc3d60f94d038fa6e0a51",
        remote = "https://github.com/tencent/rapidjson.git",
    )

    if bind:
        native.bind(
            name = "rapidjson",
            actual = "@com_github_tencent_rapidjson//:rapidjson",
        )

def abseil_repositories(bind=True):
    native.git_repository(
        name = "com_google_absl",
        commit = "787891a3882795cee0364e8a0f0dda315578d155",
        remote = "https://github.com/abseil/abseil-cpp",
    )

    if bind:
        native.bind(
            name = "abseil_strings",
            actual = "@com_google_absl//absl/strings:strings",
        )
        native.bind(
            name = "abseil_time",
            actual = "@com_google_absl//absl/time:time",
        )
    _cctz_repositories(bind)

def _cctz_repositories(bind=True):
    native.git_repository(
        name = "com_googlesource_code_cctz",
        commit = "e19879df3a14791b7d483c359c4acd6b2a1cd96b",
        remote = "https://github.com/google/cctz",
    )