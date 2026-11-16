# RiscEmu

RiscEmu, RV32I temel tamsayı komut setini C11 ile modelleyen eğitim amaçlı bir RISC-V emülatörüdür.

Proje şu anda vize aşamasındaki emülatör çekirdeğinin geliştirilme sürecindedir. Planlanan çekirdek; register dosyası, program sayacı, little-endian bellek, RV32I decoder, komut yürütme döngüsü ve otomatik testleri kapsar. ELF yükleyici, minimal sistem çağrıları ve etkileşimli debugger sonraki aşamada eklenecektir.

## Gereksinimler

- C11 uyumlu bir C derleyicisi
- GNU Make

## Derleme

```sh
make
```

Derlenen uygulama `build/rv32emu` konumunda oluşur.

## Test

```sh
make test
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

