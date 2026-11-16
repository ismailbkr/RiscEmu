# RV32I Emülatörü — Ara Rapor Taslağı

> Bu belge, uygulama tamamlandıkça gerçek test sayıları, ekran çıktıları ve gözlemlerle güncellenecek bir rapor iskeletidir. Henüz elde edilmemiş sonuçlar başarı olarak yazılmayacaktır.

## 1. Projenin amacı

Bu bitirme projesinin amacı, RISC-V mimarisinin RV32I temel tamsayı komut setini C dilinde emüle eden eğitim amaçlı bir yazılım geliştirmektir. Proje; komut getirme, çözümleme ve yürütme döngüsünü, register dosyasını ve bellek erişimlerini yazılım ortamında görünür hâle getirmeyi hedeflemektedir.

Nihai sistemin RISC-V GCC tarafından üretilen 32-bit ELF programlarını çalıştırması, minimal sistem çağrılarını desteklemesi ve kullanıcıya temel hata ayıklama komutları sağlaması planlanmaktadır.

## 2. Ara dönem hedefi

Ara dönem için projenin yüzde 60'lık işlemci çekirdeğinin tamamlanması hedeflenmiştir. Yüzde, kaynak dosya sayısı yerine testlerle doğrulanan fonksiyonel iş paketlerinden hesaplanmaktadır.

Ara dönem kapsamı şunlardır:

- Register ve program sayacı modeli.
- Little-endian bellek modeli.
- R, I, S, B, U ve J komut biçimlerinin çözülmesi.
- RV32I aritmetik, mantık, yükleme/saklama ve kontrol akışı komutları.
- Fetch–decode–execute döngüsü.
- Hata yönetimi.
- Ham makine kodu programlarıyla entegrasyon testi.
- Otomatik birim testleri.

ELF yükleme, syscall, etkileşimli debugger ve resmî uyumluluk testleri final aşamasına ayrılmıştır.

## 3. Kullanılan yöntem

Sistem birbirinden bağımsız bellek, decoder ve CPU modüllerine ayrılmıştır. Bellek modülü veri saklama ve güvenli erişimden, decoder ham 32-bit komutun alanlarını ayırmaktan, CPU ise makine durumunu ve komut yürütmeyi yönetmekten sorumludur.

Her komut aşağıdaki çevrimle çalıştırılır:

1. Program sayacının gösterdiği adresten komut alınır.
2. Opcode, register alanları ve immediate değeri çözülür.
3. İşlem yürütülür.
4. Register veya bellek güncellenir.
5. Program sayacı sıradaki komuta veya kontrol akışı hedefine taşınır.

Geliştirmede test odaklı ve aşamalı yöntem kullanılacaktır. Her modül normal durumlarla birlikte sınır ve hata koşulları açısından da sınanacaktır.

## 4. Mevcut ilerleme

Bu bölüm uygulama sırasında `docs/PROGRESS.md` verilerine göre güncellenecektir.

| Bileşen | Durum | Doğrulama |
| --- | --- | --- |
| Proje ve test altyapısı | Planlandı | — |
| Bellek | Planlandı | — |
| Decoder | Planlandı | — |
| CPU temeli | Planlandı | — |
| RV32I ALU komutları | Planlandı | — |
| Kontrol akışı | Planlandı | — |
| Load/store | Planlandı | — |
| Uçtan uca örnekler | Planlandı | — |

## 5. Teknik zorluklar

Öngörülen temel teknik zorluklar şunlardır:

- Dağınık immediate bitlerinin doğru sırada birleştirilmesi.
- Negatif immediate değerlerinde doğru işaret genişletme.
- C dilinde işaretli taşmanın tanımsız davranışından kaçınma.
- Signed ve unsigned karşılaştırmaların ayrılması.
- `x0` register'ına yapılan yazmaların etkisiz bırakılması.
- Hatalı ve hizasız bellek erişimlerinin güvenli biçimde durdurulması.

Bu riskler, sabit genişlikli tamsayı türleri, açık bit maskeleri ve sınır durumlarını içeren birim testleriyle azaltılacaktır.

## 6. Test yaklaşımı

Doğrulama üç seviyede planlanmıştır:

1. Bellek ve decoder için bağımsız birim testleri.
2. Her RV32I komutu için kontrollü başlangıç durumuyla tek-adım testleri.
3. Birden fazla komuttan oluşan küçük programlarla uçtan uca testler.

Final aşamasında bu yapıya resmî `riscv-tests` ve Spike karşılaştırması eklenecektir.

### Sonuç alanı

- Test tarihi: —
- Test komutu: —
- Toplam/geçen test: —
- Desteklenen komutlar: —
- Entegrasyon örnekleri: —
- Bilinen eksikler: —

## 7. Sonraki adımlar

Ara dönem çekirdeğinden sonra ELF32 yükleyici geliştirilecek, minimal `ecall` desteği eklenecek ve etkileşimli debugger oluşturulacaktır. Ardından resmî RV32I testleri ve Spike karşılaştırmasıyla doğruluk ölçülecek; Fibonacci, faktöriyel, sıralama, asal sayı ve matris çarpımı örnekleri çalıştırılacaktır.

## 8. Ara değerlendirme sonucu

Bu bölüm yalnızca uygulama ve test sonuçları elde edildikten sonra doldurulacaktır. Raporda tamamlanmayan bir özellik tamamlanmış gibi gösterilmeyecek; mevcut oran, doğrulanmış iş paketlerinin ağırlıklı toplamıyla belirtilecektir.

