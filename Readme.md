run.sh
```bash
#!/bin/bash
echo "----------MAKE CLEAN----------"
make clean
echo "-------------MAKE-------------"
make
echo "-------------RUN--------------"
./binary/out
```