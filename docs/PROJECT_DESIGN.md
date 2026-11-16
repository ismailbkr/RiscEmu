# RV32I Emülatörü — Proje Tasarımı

## 1. Amaç

Bu proje, RV32I temel tamsayı komut setini C11 ile emüle eden, eğitim amaçlı ve modüler bir komut seti emülatörüdür. Nihai ürün 32-bit RISC-V ELF programlarını yükleyip çalıştıracak, temel sistem çağrılarını destekleyecek ve komut yürütümünü bir hata ayıklayıcı üzerinden gözlemlemeye izin verecektir.

İlk teslim hedefi projenin ölçülebilir ve çalışır durumdaki yüzde 60'lık çekirdeğidir. Bu hedef, dosya veya satır sayısına göre değil; tamamlanmış, test edilmiş ve gösterilebilir işlevlere göre değerlendirilir.

## 2. Kapsam

### Vize aşaması — yüzde 60

- C11 tabanlı derlenebilir proje yapısı ve Makefile.
- 32 adet 32-bit register, program sayacı ve değiştirilemeyen `x0` davranışı.
- Varsayılan 1 MiB, little-endian, bayt adreslenebilir bellek.
- Byte, half-word ve word okuma/yazma işlemleri.
- Bellek sınırı ve hizalama hatalarının güvenli biçimde bildirilmesi.
- R, I, S, B, U ve J komut biçimlerinin çözülmesi.
- Immediate değerlerinin doğru oluşturulması ve işaret genişletilmesi.
- RV32I aritmetik, mantık, kaydırma, karşılaştırma, yükleme/saklama, dallanma, atlama ve üst-immediate komutları.
- Fetch–decode–execute döngüsü.
- Geçersiz komut ve yürütme hatalarında kontrollü durma.
- Küçük ham makine kodu programlarının belleğe yüklenip çalıştırılması.
- Bellek, decoder, CPU ve komut aileleri için otomatik birim testleri.

### Final aşaması — kalan yüzde 40

- ELF32 başlıklarının ve yüklenebilir program segmentlerinin okunması.
- ELF giriş adresinden yürütmenin başlatılması.
- Minimal `ecall` tabanlı çıktı ve program sonlandırma desteği.
- Etkileşimli debugger: `step`, `continue`, `break`, `regs` ve `mem`.
- Resmî `riscv-tests` entegrasyonu.
- Spike ile çıktı/durum karşılaştırması.
- Beş örnek C programı ve performans ölçümü.

### Kapsam dışı

- Pipeline, cache ve branch prediction.
- İşletim sistemi, sanal bellek ve ayrıcalık seviyeleri.
- F, D, A ve C uzantıları.
- Grafik arayüz.
- M uzantısı, zorunlu hedefler bittikten sonra isteğe bağlıdır.

## 3. Mimari

Emülatör tek çevrimli bir soyut işlemci modeli kullanır. Her CPU adımı aşağıdaki sırada yürür:

1. PC adresinden 32-bit komut okunur.
2. Komut alanları ve immediate değeri çözülür.
3. İlgili RV32I işlemi yürütülür.
4. Register veya bellek durumu güncellenir.
5. Dallanma/atlama yoksa PC dört artırılır.
6. `x0` register'ı sıfıra sabitlenir.
7. Hata veya durma koşulu üst katmana bildirilir.

Bağımlılık yönü aşağıdaki gibidir:

```text
main / testler
      |
      v
     cpu ------> decoder
      |
      v
    memory
```

`memory` ve `decoder` CPU'dan bağımsızdır. CPU bu modülleri kullanır; modüller CPU'nun iç durumuna erişmez. Bu sınır, bileşenlerin bağımsız test edilmesini sağlar.

## 4. Dosya yapısı

```text
RiscEmu/
├── Makefile
├── README.md
├── docs/
│   ├── PROJECT_DESIGN.md
│   ├── IMPLEMENTATION_PLAN.md
│   ├── PROGRESS.md
│   └── INTERIM_REPORT_NOTES.md
├── include/
│   ├── cpu.h
│   ├── decoder.h
│   ├── memory.h
│   └── rv32_types.h
├── src/
│   ├── main.c
│   ├── cpu.c
│   ├── decoder.c
│   └── memory.c
├── tests/
│   ├── test_cpu.c
│   ├── test_decoder.c
│   ├── test_instructions.c
│   ├── test_memory.c
│   └── test_runner.c
└── examples/
    └── raw/
```

## 5. Modül sözleşmeleri

### Bellek

Bellek modülü ayrılmış bir byte dizisinin sahipliğini taşır. Oluşturma/yok etme, temizleme ve 8/16/32-bit okuma-yazma işlemlerini sunar. Her erişim, işlem yapılmadan önce adres aralığını ve gerekli hizalamayı doğrular. Hatalar süreçten çıkmak yerine durum koduyla çağırana döner.

### Decoder

Decoder, 32-bit komutu salt veri niteliğindeki bir çözümleme sonucuna dönüştürür. Sonuçta opcode, `rd`, `rs1`, `rs2`, `funct3`, `funct7` ve ilgili immediate alanları bulunur. Decoder register veya belleği değiştirmez.

### CPU

CPU; register dosyası, PC, bellek bağlantısı, çalışma/durma durumu ve son hata bilgisini tutar. Tek komut çalıştırma işlevi, debugger ve normal çalışma döngüsünün ortak giriş noktasıdır. Komut yürütme opcode ailelerine göre küçük iç fonksiyonlara ayrılır.

### Ana program

Vize aşamasında ana program ham little-endian komut dosyasını belirlenen başlangıç adresine yükler ve bir komut sınırıyla çalıştırır. Komut sınırı, sonsuz döngülerin kontrolsüz çalışmasını engeller. ELF desteği geldiğinde yükleme mekanizması değişecek, CPU arayüzü korunacaktır.

## 6. RV32I komut kapsamı

- U tipi: `LUI`, `AUIPC`
- J tipi: `JAL`
- I tipi atlama: `JALR`
- B tipi: `BEQ`, `BNE`, `BLT`, `BGE`, `BLTU`, `BGEU`
- Load: `LB`, `LH`, `LW`, `LBU`, `LHU`
- Store: `SB`, `SH`, `SW`
- Immediate ALU: `ADDI`, `SLTI`, `SLTIU`, `XORI`, `ORI`, `ANDI`, `SLLI`, `SRLI`, `SRAI`
- Register ALU: `ADD`, `SUB`, `SLL`, `SLT`, `SLTU`, `XOR`, `SRL`, `SRA`, `OR`, `AND`
- Sistem: Vize aşamasında `ECALL` kontrollü durma olarak tanınabilir; syscall davranışı final aşamasındadır.
- `FENCE`: Tek çekirdekli ve harici cihazsız modelde no-op olarak ele alınır.

## 7. Hata modeli

Kütüphane niteliğindeki modüller `exit()` çağırmaz. Hatalar enum durum kodlarıyla iletilir. Hata sınıfları en az şunları içerir:

- Bellek sınırı dışında erişim.
- Hizasız half-word veya word erişimi.
- Hizasız instruction fetch.
- Geçersiz veya desteklenmeyen komut kodlaması.
- Program yükleme hatası.
- Kullanıcı tarafından belirlenen komut sınırının aşılması.

CPU hata aldığında durur, PC ve ham komut korunur; böylece test ve ilerideki debugger anlamlı tanılama sunabilir.

## 8. Doğrulama stratejisi

Her modül bağımsız test edilir. Komut testleri başlangıç register/bellek durumunu kurar, tek adım çalıştırır ve sonuç durumu ile PC'yi doğrular. Sınır değerleri özellikle test edilir:

- `0`, `-1`, `INT32_MIN`, `INT32_MAX`.
- Signed/unsigned karşılaştırma farkları.
- Negatif branch ve jump offsetleri.
- Shift miktarının düşük beş biti.
- Signed ve unsigned load işaret genişletmesi.
- `x0` hedefli yazma girişimleri.
- Taşmalı 32-bit aritmetiğin modulo 2^32 davranışı.

Vize kalite kapısı: temiz derleme, tüm yerel testlerin geçmesi ve en az iki ham örnek programın beklenen son register/bellek durumuna ulaşmasıdır.

## 9. Tasarım kararları

- Harici çalışma zamanı kütüphanesi kullanılmaz.
- İç durumlarda sabit genişlikli `stdint.h` türleri kullanılır.
- İşaretli taşmaya güvenilmez; aritmetik mümkün olduğunca `uint32_t` üzerinde yürütülür.
- Immediate üretimi bit maskeleri ve açık işaret genişletme yardımcılarıyla yapılır.
- Global değişken kullanılmaz.
- Bir komutun decode ve execute sorumlulukları ayrıdır.
- Vize aşamasında ELF'e erken bağlanmak yerine önce işlemci çekirdeğinin doğruluğu kanıtlanır.

## 10. Tamamlanma tanımı

Bir iş yalnızca kod yazıldığında değil, aşağıdakilerin tümü sağlandığında tamamlanmış sayılır:

1. İlgili arayüz ve davranış uygulanmıştır.
2. Normal ve hata senaryoları test edilmiştir.
3. Derleyici uyarısı yoktur.
4. Tüm test paketi geçmektedir.
5. İlerleme belgesindeki kanıt alanı güncellenmiştir.

