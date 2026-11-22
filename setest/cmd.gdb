file setest
b gen_suite_02_fixstar.c:21 if ctx->current.iteration.id == 1
r t/mytest -s 2.1.1
p *ctx->reader->block[1].entry[0].value
