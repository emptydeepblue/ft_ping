# Ping 

1. **Command: `ping`**

    Error(1) thrown - in parsing: destination address required

    ```
    ping: usage error: Destination address required
    ```

## ping single destination
2. **Commands:** 
    - *`ping .com`* 
    - *`ping ,com`* 
    - *`ping ..com`* 
    - *`ping +com`* 
    - *`ping ''`*
   
    Error(2) thrown - in parsing: unknown service

    ```
    ping: .com: Name or service not known
    ```
 <!-- - *`ping pingé.com`*  same result as above but takes more time -->

3. **Commands:** 
    - *`ping q`*
    - *`ping google.c0m`* 
    - *`ping 127.0.0.1extra`*

    Error(2) thrown - in DNS resolution: unknown service

    ```
    ping: google.c0m: Name or service not known
    ```
4. **Commands:**
    - *`ping 1`* 
    - *`ping 999.999.999.999`*

    (0) No error thrown, No response received

    ```
    PING 1 (0.0.0.1) 56(84) bytes of data.
    ^C
    --- 1 ping statistics ---
    28 packets transmitted, 0 received, 100% packet loss, time 28110ms
    ```
5. **Commands:** 
    - *`ping 127.0.0.1`* 
    - *`ping google.com`*

    (0) No error thrown, Response received
    ```
    PING 127.0.0.1 (127.0.0.1) 56(84) bytes of data.
    64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=0.089 ms
    64 bytes from 127.0.0.1: icmp_seq=2 ttl=64 time=0.037 ms
    64 bytes from 127.0.0.1: icmp_seq=3 ttl=64 time=0.045 ms
    64 bytes from 127.0.0.1: icmp_seq=4 ttl=64 time=0.035 ms
    64 bytes from 127.0.0.1: icmp_seq=5 ttl=64 time=0.046 ms
    ^C
    --- 127.0.0.1 ping statistics ---
    5 packets transmitted, 5 received, 0% packet loss, time 4117ms
    rtt min/avg/max/mdev = 0.035/0.050/0.089/0.019 ms
    ```

6. **Command: `ping 💀.com`**

    Error(2) thrown - in parsing: string not correctly encoded

    ```
    ping: 💀.com: Parameter string not correctly encoded
    ```
## ping multiple destinations

6. **Commands:**
    - *`ping test 127.0.0.1`* 
    - *`ping 127.0.0.1 test`* 
    - *`ping 120.0.0.1 120.0.0.2 test`*

    Error(2) thrown - in DNS resolution: failure in name resolution

    ```
    ping: test: Temporary failure in name resolution
    ```

7. **Commands:** 
    - *`ping 127.0.0.1 127.0.0.2`*
    - *`ping 127.0.0.2 127.0.0.3`*
    - *`ping 120.0.0.1 120.0.0.2 120.0.0.3`*

    (0) No error thrown, No response received (try with the last)

    ```
    PING 127.0.0.3 (127.0.0.3) 56(124) bytes of data.
    ```
## ping with options

8. **Commands:**
    - *`ping -`*
    - *`ping 127.0.0.1 -`*

    Error(2) thrown, in parsing: unknown service
    ```
    ping: -: Name or service not known
    ```

9. **Command: `ping - 127.0.0.1`**

    Error(2) thrown - in DNS resolution: failure in name resolution
    ```
    ping: -: Temporary failure in name resolution
    ```

10. **Commands:**
    - *`ping -E`*
    - *`ping -e`*
    - *`ping --E`* 
    - *`ping --e`*
    - *`ping -e 127.0.0.1`* 
    - *`ping 127.0.0.1 -e`*

    Error(2) thrown - in parsing: invalid option, help displayed
    ```
    ping: invalid option -- 'e'

    Usage
    ping [options] <destination>

    Options:
        <destination>      dns name or ip address
        -v                 verbose output

    For more details see ping(8).
    ```

### ping with help option

11. **Commands:**
    - *`ping -?`* 
    - *`ping -? -?`*
    - *`ping -??`*

    (2) No error thrown, help displayed
    ```
    Usage
    ping [options] <destination>

    Options:
    <destination>      dns name or ip address
    -v                 verbose output

    For more details see ping(8).
    ```
12. **Command:** 
    - *`ping 999.999.999.999 -?`* 
    - *`ping -? 999.999.999.999`*
    - *`ping -? 127.0.0.1`* 
    - *`ping 127.0.0.1 -?`*
    - *`ping 127.0.0.1 test -?`* 
    - *`ping test 127.0.0.1 test -? test`*

    (2) No error thrown, help displayed
    ```
    Usage
    ping [options] <destination>

    Options:
    <destination>      dns name or ip address
    -v                 verbose output

    For more details see ping(8).
    ```
13. **Commands:** 
    - *`ping -?test`*
    - *`ping -?127.0.0.1`*
    
    (2) No error thrown, help displayed
    ```
    Usage
    ping [options] <destination>

    Options:
    <destination>      dns name or ip address
    -v                 verbose output

    For more details see ping(8).
    ```

### ping with verbose option

14. **Commands:** 
    - *`ping -v`*
    - *`ping -vv`*
    - *`ping -v -v`*

    Error(1) thrown, in parsing: destination address required
    ```
    ping: usage error: Destination address required
    ```
15. **Commands:** 
    - *`ping -v 127.0.0.1 test`*
    - *`ping 127.0.0.1 test -v`*

    Error(2) thrown in DNS resolution: failure in name resolution
    ```
    ping: test: Temporary failure in name resolution
    ```
16. **Commands:** 
    - *`ping -v 127.0.0.1 127.0.0.2`*
    - *`ping 127.0.0.1 127.0.0.2 -v`*

    No error thrown, no response received
    ```
    PING 127.0.0.2 (127.0.0.2) 56(124) bytes of data.
    ^C
    --- 127.0.0.2 ping statistics ---
    4 packets transmitted, 0 received, 100% packet loss, time 3202ms
    ```
17. **Commands:** 
    - *`ping 127.0.0.1 -v`*
    - *`ping -v 127.0.0.1`*
    - *`ping 127.0.0.1 -vv`*
    - *`ping 127.0.0.1 -v -v`*

    No error thrown, verbose response received
    ```
    PING 127.0.0.1 (127.0.0.1) 56(84) bytes of data.
    64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=0.060 ms
    64 bytes from 127.0.0.1: icmp_seq=2 ttl=64 time=0.047 ms
    64 bytes from 127.0.0.1: icmp_seq=3 ttl=64 time=0.048 ms
    ^C
    --- 127.0.0.1 ping statistics ---
    3 packets transmitted, 3 received, 0% packet loss, time 2174ms
    rtt min/avg/max/mdev = 0.047/0.051/0.060/0.005 ms
    ```

### ping with both options

18. **Commands:** 
   - *`ping -? -v`*
   - *`ping -v -?`*

    (2) No error thrown, help displayed
    ```
    Usage
    ping [options] <destination>

    Options:
    <destination>      dns name or ip address
    -v                 verbose output

    For more details see ping(8).
    ```

19. **Command:** 
   - *`ping -? 127.0.0.1 -v`* 
   - *`ping -v 127.0.0.1 -?`* 
   - *`ping 127.0.0.1 -v -?`*
   - *`ping 127.0.0.1 -?v`*
   - *`ping 127.0.0.1 -v?`*

    (2) No error thrown, help displayed
    ```
    Usage
    ping [options] <destination>

    Options:
    <destination>      dns name or ip address
    -v                 verbose output

    For more details see ping(8).
    ```
    
