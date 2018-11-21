// NOLINT(namespace-envoy)
#include "test/test_common/environment.h"
#include "test/test_runner.h"

#if defined (__HAS_FF__)
#include "ff_api.h"
#endif

#include "absl/debugging/symbolize.h"

#ifdef ENVOY_HANDLE_SIGNALS
#include "exe/signal_action.h"
#endif

#if defined (__HAS_FF__)
#define FF_CONFIG_NUM   5
//extern int ff_init(int argc, char * const argv[]);
char    *ff_argv[FF_CONFIG_NUM + 1];
int     ff_argc;
#endif


const char* __asan_default_options() {
  static char result[] = {"check_initialization_order=true strict_init_order=true"};
  return result;
}

// The main entry point (and the rest of this file) should have no logic in it,
// this allows overriding by site specific versions of main.cc.
int main(int argc, char** argv) {
#ifndef __APPLE__
  absl::InitializeSymbolizer(argv[0]);
#endif
#ifdef ENVOY_HANDLE_SIGNALS
  // Enabled by default. Control with "bazel --define=signal_trace=disabled"
  Envoy::SignalAction handle_sigs;
#endif

  ::setenv("TEST_RUNDIR",
           (Envoy::TestEnvironment::getCheckedEnvVar("TEST_SRCDIR") + "/" +
            Envoy::TestEnvironment::getCheckedEnvVar("TEST_WORKSPACE"))
               .c_str(),
           1);

  std::string dpdk_cfg_file = Envoy::TestEnvironment::getCheckedEnvVar("TEST_DPDK_CFG");

  // Select whether to test only for IPv4, IPv6, or both. The default is to
  // test for both. Options are {"v4only", "v6only", "all"}. Set
  // ENVOY_IP_TEST_VERSIONS to "v4only" if the system currently does not support IPv6 network
  // operations. Similarly set ENVOY_IP_TEST_VERSIONS to "v6only" if IPv4 has already been
  // phased out of network operations. Set to "all" (or don't set) if testing both
  // v4 and v6 addresses is desired. This feature is in progress and will be rolled out to all tests
  // in upcoming PRs.
  ::setenv("ENVOY_IP_TEST_VERSIONS", "all", 0);

  int n = 0;

 // name of binary
 ff_argv[n++] = strdup("exe");

 // -c switch for config.ini
 ff_argv[n++] = strdup("-c");

 // location to config.ini
 // use an environment variable (getCheckedEnvVar) to read the location of config file
 ff_argv[n++] = strdup(dpdk_cfg_file.c_str());
  
 // --proc-type=primary OR --proc-type=secondary
 std::string proc_type("--proc-type=primary");
 ff_argv[n++] = strdup(proc_type.c_str());
  
 // --proc-id=0 or --proc-id=1 etc.
 std::string proc_id("--proc-id=" + std::to_string(0));
 ff_argv[n++] = strdup(proc_id.c_str());

 ff_argc = n;
 ff_init(ff_argc, ff_argv);

  return Envoy::TestRunner::RunTests(argc, argv);
}
