# RV32I Emülatörü — Uygulama Planı

## Hedef

Birinci çalışma diliminde, vize ara raporunda gösterilebilecek ve testlerle kanıtlanabilecek emülatör çekirdeğini tamamlamak. Uygulama sırası, her aşamanın bir sonraki aşama için doğrulanmış temel oluşturmasını sağlar.

## Çalışma kuralları

- Her aşama küçük ve bağımsız değişikliklerden oluşur.
- Önce beklenen davranışı gösteren test, sonra uygulama yazılır.
- Her aşama sonunda `make test` ve temiz derleme çalıştırılır.
- Başarısız test varken sonraki aşamaya geçilmez.
- İlerleme ve test kanıtları `docs/PROGRESS.md` dosyasına işlenir.
- Mevcut `practice/` dosyaları öğrenme örneği olarak korunur; üretim kaynaklarına bağlanmaz.

## Aşama 0 — Depo güvenliği ve iskelet

**Amaç:** Projeyi bağımsız, tekrar üretilebilir bir C11 yapısına hazırlamak.

**İşler:**

1. Mevcut üst dizin Git durumunu incelemek ve RiscEmu için güvenli depo sınırını belirlemek.
2. `src`, `include`, `tests` ve `examples/raw` dizinlerini oluşturmak.
3. Katı uyarıları etkinleştiren Makefile hazırlamak.
4. Test çalıştırıcısını ve ilk smoke testini eklemek.
5. README'ye derleme ve test komutlarını yazmak.

**Kalite kapısı:** `make`, `make test` ve `make clean` beklenen şekilde çalışır; derleyici uyarısı yoktur.

## Aşama 1 — Ortak türler ve bellek

**Amaç:** CPU'dan bağımsız, güvenli little-endian bellek modeli.

**İşler:**

1. Ortak durum kodlarını tanımlamak.
2. Bellek oluşturma, sıfırlama ve yok etme işlemlerini eklemek.
3. 8/16/32-bit okuma-yazma arayüzlerini eklemek.
4. Sınır ve hizalama kontrollerini uygulamak.
5. Little-endian düzeni ve hata durumlarını test etmek.

**Kalite kapısı:** Bellek normal, sınır ve hizalama testlerinin tamamını geçer.

## Aşama 2 — Decoder

**Amaç:** Altı RV32I biçimini yan etkisiz biçimde çözmek.

**İşler:**

1. Opcode ve register alanlarını çıkarmak.
2. `funct3` ve `funct7` alanlarını çıkarmak.
3. I, S, B, U ve J immediate değerlerini üretmek.
4. İşaret genişletme yardımcısını eklemek.
5. Pozitif/negatif sınır offsetleriyle decoder testleri yazmak.

**Kalite kapısı:** Bilinen komut kodlamaları bütün alanlar için doğru çözülür.

## Aşama 3 — CPU temeli

**Amaç:** Register dosyası, PC ve tek-adım yürütme omurgası.

**İşler:**

1. CPU oluşturma/sıfırlama arayüzü.
2. Kontrollü register okuma-yazma ve `x0` koruması.
3. Hizalı instruction fetch.
4. Tek adım ve sınırlı çalışma döngüsü.
5. Hata/durma durumunun saklanması.

**Kalite kapısı:** PC ilerlemesi, fetch hataları, `x0` ve komut sınırı testleri geçer.

## Aşama 4 — Üst immediate ve ALU komutları

**Amaç:** Hesaplama komutlarının tamamı.

**İşler:**

1. `LUI`, `AUIPC`.
2. Immediate ALU komutları.
3. Register ALU komutları.
4. Shift kodlamalarının geçerlilik kontrolü.
5. Signed/unsigned karşılaştırma ve taşma testleri.

**Kalite kapısı:** Her komut için en az bir normal ve bir sınır senaryosu geçer.

## Aşama 5 — Kontrol akışı

**Amaç:** Branch ve jump davranışlarının eksiksiz uygulanması.

**İşler:**

1. Altı branch komutu.
2. `JAL` ve `JALR`.
3. Alınan/alınmayan branch testleri.
4. Pozitif/negatif hedef ve dönüş adresi testleri.
5. Hizasız hedef hata testi.

**Kalite kapısı:** PC ve link register sonuçları tüm kontrol akışı testlerinde doğrudur.

## Aşama 6 — Load/store

**Amaç:** CPU ile bellek arasında doğru veri aktarımı.

**İşler:**

1. `LB`, `LH`, `LW`, `LBU`, `LHU`.
2. `SB`, `SH`, `SW`.
3. Effective address hesaplama.
4. İşaret genişletme testleri.
5. Hatalı adres işlemlerinin CPU'yu güvenle durdurması.

**Kalite kapısı:** Tüm load/store testleri ve bellek hata yayılımı geçer.

## Aşama 7 — Sistem kodlamaları ve uçtan uca çalışma

**Amaç:** Çekirdeği küçük programları çalıştırabilen bir ürüne dönüştürmek.

**İşler:**

1. `FENCE` no-op davranışı.
2. `ECALL` için vize aşaması kontrollü durma davranışı.
3. Ham binary yükleyici.
4. CLI argümanları ve komut sınırı.
5. En az iki örnek makine kodu programı.
6. Son register/bellek durumunu doğrulayan entegrasyon testleri.

**Kalite kapısı:** Örnek programlar deterministik biçimde beklenen sonucu üretir.

## Aşama 8 — Belgeleme ve vize kanıt paketi

**Amaç:** Yapılan işi ara raporda ölçülebilir biçimde sunmak.

**İşler:**

1. README kullanım örnekleri.
2. Modül ve veri akışı açıklaması.
3. Komut destek matrisi.
4. Test sonuçlarının tabloya işlenmesi.
5. Ekran görüntüsü alınabilecek demo komutlarının hazırlanması.
6. Ara rapor metninin gerçek sonuçlarla tamamlanması.

**Kalite kapısı:** Temiz ortamdan derleme/test adımları tekrarlanabilir ve rapordaki her tamamlanma iddiasının bir kanıtı vardır.

## Sonraki çalışma dilimi — Final yüzde 40

1. ELF32 doğrulama ve segment yükleme.
2. Minimal syscall ABI'sinin tanımlanması ve uygulanması.
3. Debugger komut döngüsü ve breakpointler.
4. RISC-V toolchain ile örneklerin derlenmesi.
5. `riscv-tests` adaptörü ve test sonuçları.
6. Spike karşılaştırması.
7. Beş C örneği ve performans raporu.

## İş bölümü ve entegrasyon

Bağımsız modüller gerektiğinde alt ajanlara verilebilir. Bellek, decoder ve test senaryoları paralel çalışmaya uygundur. CPU entegrasyonu, ortak türler ve genel mimari tek sorumlu tarafından birleştirilir. Her alt çalışma aynı arayüz sözleşmelerini kullanmalı ve ana test paketiyle doğrulanmadan tamamlanmış kabul edilmemelidir.

## Geri alma stratejisi

Her aşama küçük ve izole tutulur. Bir aşama kalite kapısını geçmezse sonraki aşama başlamaz; başarısız değişiklik düzeltilir veya son doğrulanmış duruma dönülür. Kullanıcıya ait mevcut dosyalar silinmez veya üzerine yazılmaz.

