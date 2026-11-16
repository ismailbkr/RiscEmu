# RiscEmu

RiscEmu, RV32I temel tamsayı komut setini C11 ile modelleyen eğitim amaçlı bir RISC-V emülatörüdür.

Vize aşamasındaki emülatör çekirdeği tamamlanmıştır. Çekirdek; register dosyası, program sayacı, little-endian bellek, RV32I decoder, temel komut setinin yürütülmesi, ham binary yükleme ve otomatik testleri kapsar. ELF yükleyici, minimal sistem çağrıları ve etkileşimli debugger sonraki aşamada eklenecektir.

## Gereksinimler

- C11 uyumlu bir C derleyicisi
- GNU Make

## Derleme

```sh
make
```

Derlenen uygulama `build/rv32emu` konumunda oluşur.

## Ham program çalıştırma

Program, little-endian RV32I komutlarından oluşan ve adres `0`'a yüklenecek ham bir binary kabul eder:

```sh
./build/rv32emu program.bin
./build/rv32emu --max-steps 10000 program.bin
```

Çalışma sonunda durma nedeni, PC, yürütülen komut sayısı ve 32 register gösterilir. Vize sürümünde `ECALL` kontrollü durma işaretidir; gerçek syscall davranışı final aşamasında eklenecektir.

## Test

```sh
make test
```

Mevcut test paketi bellek, decoder, CPU, tüm RV32I komut aileleri, hata durumları ve iki ham entegrasyon programını kapsar.

## Örnek programlar

Tekrarlanabilir iki `.bin` örneğini üretip çalıştırmak için:

```sh
make examples
./build/rv32emu examples/raw/addi_ecall.bin
./build/rv32emu examples/raw/sum_loop.bin
```

## Temizleme

```sh
make clean
```

## Belgeler

- `docs/PROJECT_DESIGN.md`: Mimari ve kapsam
- `docs/IMPLEMENTATION_PLAN.md`: Aşamalı geliştirme planı
- `docs/PROGRESS.md`: Ağırlıklı ilerleme ve kanıt tablosu
- `docs/INTERIM_REPORT_NOTES.md`: Ara rapor taslağı

## Başlangıç örnekleri

`practice/` dizinindeki küçük programlar öğrenme denemeleridir ve üretim emülatörünün parçası değildir.

