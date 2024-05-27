### wrk -t4 -c64 -d60s -s get.lua http://127.0.0.1:8888
```
Running 1m test @ http://127.0.0.1:8888
  4 threads and 64 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    20.36ms  102.27ms   1.24s    97.54%
    Req/Sec     0.93k   686.18     2.79k    50.59%
  150085 requests in 1.00m, 36.87MB read
  Non-2xx or 3xx responses: 84818
Requests/sec:   2498.66
Transfer/sec:    628.58KB
```

### wrk -t4 -c64 -d60s -s get.lua http://127.0.0.1:8888
```
Running 1m test @ http://127.0.0.1:8888
  4 threads and 64 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     2.22ms    8.50ms 373.17ms   99.84%
    Req/Sec     2.70k     1.89k    8.43k    53.77%
  480725 requests in 1.00m, 105.16MB read
Requests/sec:   8002.60
Transfer/sec:      1.75MB
```