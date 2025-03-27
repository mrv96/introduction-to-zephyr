Build:

```sh
west build -p always -b b_u585i_iot02a -- -DEXTRA_CONF_FILE=boards/b_u585i_iot02a.conf
```

Modify Kconfig:

```sh
west build -p always -b b_u585i_iot02a -t menuconfig -- -DEXTRA_CONF_FILE=boards/b_u585i_iot02a.conf
```
