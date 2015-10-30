# udrawd
HTTP/2 tile server for transmitting PNG tiles for drawing onto an extended HTML5 canvas.
* Currently uses Redis  for storage
* HTTPS only via HTTP/2 w/ ALPN enabled on OpenSSL 1.0.2+
* HTTP Push JavaScript, fonts & CSS resources
* C++11
* nghttp2 boost asio library
#### nghttp2 dependency install
##### Mac using Hombrew

```brew install nghttp2```

##### Ubuntu 15.10
In order to build, install dependencies for nghttp2 then compile and install.
```
sudo apt-get install g++ make binutils autoconf automake autotools-dev libtool pkg-config zlib1g-dev libcunit1-dev libssl-dev libxml2-dev libev-dev libevent-dev libjansson-dev libjemalloc-dev cython python3-dev python-setuptools libboost-system-dev libboost-thread-dev libhiredis-dev
```
Building is then fairly easy
```
git clone https://github.com/tatsuhiro-t/nghttp2.git
cd nghttp2
git checkout v1.4.0
autoreconf -i
automake
autoconf
./configure --enable-asio-lib
make 
```
## Build udrawd
```
git clone https://tsullivan@bitbucket.org/tsullivan/udrawd.git
cd udrawd
make
```
#### Creating Certificates
```
openssl req -nodes -x509 -newkey rsa:2048 -sha256 -keyout serverkey.pem -out servercert.pem -days 365
```
## Running udrawd
The webroot specifies a path to serve static files which match the url ```/static/*```
```
./udrawd <serverkey.pem> <servercert.pem> <wwwroot (eg. './html')>
```
