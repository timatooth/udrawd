# udrawd
HTTP/2 tile server for serving PNG tiles for drawing onto an extended HTML5 canvas.
* Currently uses Redis for storage
* HTTPS only via HTTP/2 w/ ALPN enabled on OpenSSL 1.0.2+
* C++11
* nghttp2 Boost Asio library

##### Mac using Homebrew

```brew install cmake nghttp2 hiredis openssl boost```

##### Ubuntu
In order to build, install dependencies for nghttp2 then compile and install.
```
sudo apt-get install g++ make binutils autoconf automake autotools-dev libtool pkg-config zlib1g-dev libcunit1-dev libssl-dev libxml2-dev libev-dev libevent-dev libjansson-dev libjemalloc-dev cython python3-dev python-setuptools libboost-system-dev libboost-thread-dev libhiredis-dev
```
Building nghttp2 is fairly easy
```
git clone https://github.com/tatsuhiro-t/nghttp2.git
cd nghttp2
git checkout v1.23.0
autoreconf -i
automake
autoconf
./configure --enable-asio-lib
make
sudo make install
```
## Build udrawd
```
cmake .
make
```

#### Creating Self-Signed Certificates for Testing
```
openssl req -nodes -x509 -newkey rsa:2048 -sha256 -keyout serverkey.pem -out servercert.pem -days 365
```
## Running udrawd
The docroot specifies a path to serve static files which match the url ```/static/*```.
```index.html``` will be served when ```/``` is requested and will try open <docroot>/static/index.html.
```
./udrawd <serverkey.pem> <servercert.pem> <docroot>
```
