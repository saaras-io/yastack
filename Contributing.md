# Contribution guidelines

Thanks for taking the time to join our community. 
These guidelines will help you get started with the YAStack project.
Contributions to YAStack project require a Developer Certificate of Origin (see below)

## How to make your contributions

This section describes the process for contributing a bug fix or new feature.

Before submitting a pull request for a feature, please [create an issue][1] and
discuss it with the community. This ensures that no one else is working on it.

### Notes for testing a change

To verify your change by deploying the image you built, you need to run it on
a machine that has atleast one network interfaces supported by DPDK. Please be
careful that the support for KNI is not yet tested. So, it is required to test
the changes on a machine with atleast two interfaces - one interface is used for
managment traffic and accessing the instance and the other interface is used for
data traffic (and runs the igb_uio kernel module)

## Developer Certificate of Origin

We require that -
(1) you agree to the standard text in the Developer Certificate of Origin 1.1
(2) you sign your commits using git commit --signoff

The sign-off is a simple line at the end of the commit message for the patch or
pull request, which certifies that you wrote it or otherwise have the right to 
pass it on as an open-source patch. The rules are pretty simple: if you can 
certify what is said in the Developer Certificate of Origin 1.1 (see also below) 
then you just add a line saying:

```
Signed-off-by: YAStack Developer <yastack@developer.example.random.com>
```

When committing using the command line you can sign off using the --signoff 
or -s flag. This adds a Signed-off-by line by the committer at the end of the 
commit log message.

By doing this you state that you can certify the following (from https://developercertificate.org/):

```
Developer Certificate of Origin
Version 1.1

Copyright (C) 2004, 2006 The Linux Foundation and its contributors.
1 Letterman Drive
Suite D4700
San Francisco, CA, 94129

Everyone is permitted to copy and distribute verbatim copies of this
license document, but changing it is not allowed.


Developer's Certificate of Origin 1.1

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the open source license
    indicated in the file; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part
    by me, under the same open source license (unless I am
    permitted to submit under a different license), as indicated
    in the file; or

(c) The contribution was provided directly to me by some other
    person who certified (a), (b) or (c) and I have not modified
    it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    personal information I submit with it, including my sign-off) is
    maintained indefinitely and may be redistributed consistent with
    this project or the open source license(s) involved.
```

[1]: https://github.com/saaras-io/yastack/issues
