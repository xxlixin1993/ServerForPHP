# ServerForPHP
A web server for php-fpm

## flow
```flow
st=>start: Listen socket, wait for connect
op=>operation: read in request
cond=>condition: GET or POST?
cond2=>condition: Dynamic requests(yes) or static requests(no)?
op2=>operation: connect php-fpm
op3=>operation: send fastcgi param data
op4=>operation: handle php-fpm response, return client
e=>end

st->op->cond->cond2->op2->op3->op4->e
cond(yes)->cond2
cond(no)->e
cond2(yes)->op2
cond2(no)->e
```