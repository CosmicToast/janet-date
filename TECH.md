# Reading Guide
A general discussion on time, and why there isn't enough of it to discuss it.

ISO C99 is fairly restrictive, intending to only really work with localtime.
This assumption goes fairly deep, to the point of the macos libc (for example)
presuming that all strftime calls are against localtime.

Thankfully, IANA tzcode is more reasonable, as it handles things correctly
and provides timegm (which means you can operate in UTC until final display).

On the overall, the package wraps around all ISO C99 functions that are not
marked for future deprecation, plus timegm.
The native wrapping is optimized for working primarily with time_t, only really
using struct tm for final output and intermediate representations.
This is how it's recommended to use the library as well.

The primary thing missing is the ability to represent an arbitrary-timezone.
For example, representing EST while localtime is CET.
Unfortunately, ISO C99 simply does not provide a way of doing this.
IANA tzcode does have ways to do this, namely via the NetBSD-inspired
`_z`-terminated functions.
Unfortunately, making those usable without completely messing up the codebase
is non-trivial.

Anyway, if nothing else, something really fun that came out of this project
is that I got to report a libc bug to apple, so that's cool.
It's been almost a month and they don't seem to have acknowledged it, so if you
have to work with time, I would recommend just using IANA tzcode.
It's literally standard.
