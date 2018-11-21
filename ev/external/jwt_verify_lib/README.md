
This repository stores JWT verification files for c++.
These files are originally created in [istio/proxy jwt_auth folder](https://github.com/istio/proxy/blob/master/src/envoy/http/jwt_auth/jwt.h).
The key reason to create a separate repo for them is that they can be used by other projects. For example, [envoyproxy](https://github.com/envoyproxy/envoy) likes to use these code to build a jwt_auth HTTP filter.

This is not an officially supported Google product

For contributors:
If you make any changes, please make sure to use Bazel to pass all unit tests by running:

```
bazel test //...
```
Please format your codes by running:

```
script/check-style
```


