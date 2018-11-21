
nghttp2_option_set_peer_max_concurrent_streams
==============================================

Synopsis
--------

*#include <nghttp2/nghttp2.h>*

.. function:: void nghttp2_option_set_peer_max_concurrent_streams(nghttp2_option *option, uint32_t val)

    
    This option sets the SETTINGS_MAX_CONCURRENT_STREAMS value of
    remote endpoint as if it is received in SETTINGS frame.  Without
    specifying this option, before the local endpoint receives
    SETTINGS_MAX_CONCURRENT_STREAMS in SETTINGS frame from remote
    endpoint, SETTINGS_MAX_CONCURRENT_STREAMS is unlimited.  This may
    cause problem if local endpoint submits lots of requests initially
    and sending them at once to the remote peer may lead to the
    rejection of some requests.  Specifying this option to the sensible
    value, say 100, may avoid this kind of issue. This value will be
    overwritten if the local endpoint receives
    SETTINGS_MAX_CONCURRENT_STREAMS from the remote endpoint.
