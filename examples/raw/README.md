# Ham RV32I örnekleri

İki tekrarlanabilir örnek üretmek için proje kökünde aşağıdaki komutu çalıştırın:

```sh
make examples
```

Bu komut şunları oluşturur:

- `addi_ecall.bin`: `x1` register'ına 5 yazar ve `ECALL` ile durur.
- `sum_loop.bin`: 1–10 toplamını `x1` register'ında hesaplar, sonucu bellek adresi 256'ya yazar ve `ECALL` ile durur.

Çalıştırma:

```sh
./build/rv32emu examples/raw/addi_ecall.bin
./build/rv32emu examples/raw/sum_loop.bin
```
