file setest
b generated_tests.c:232 if ctx->current.iteration.id == 1
r t/mytest -s 2.1.1
p *ctx->reader->block[1].entry[0].value
