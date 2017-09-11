# ServerForPHP
[![Build Status](https://travis-ci.org/xxlixin1993/ServerForPHP.svg?branch=master)](https://travis-ci.org/xxlixin1993/ServerForPHP)

A web server for php-fpm

## Requirement
- CMake 3.0 +
- gcc 4.8.4 +

## Install
### Compile in Linux
```
cd /path/to/ServerForPHP
bash travis/compile.sh
```

### Run

1. `/path/to/ServerForPHP/bin/ServerForPHP`
2. curl 127.0.0.1:8000
```
lixin@lixin-PC:~/ServerForPHP$ curl 127.0.0.1:8000
<h1>Welcome</h1>
```

## flow
![flow](readme_img/server_for_php.png)
