# RV32I Emülatörü — İlerleme Takibi

## Değerlendirme yöntemi

İlerleme yüzdesi satır sayısına göre değil, doğrulanmış iş paketlerinin ağırlığına göre hesaplanır. Bir kalem ancak uygulama, test ve belge kanıtı birlikte mevcutsa tamamlanır.

## Vize hedefi

| İş paketi | Proje ağırlığı | Durum | Kanıt |
| --- | ---: | --- | --- |
| Proje yapısı, Makefile ve test altyapısı | %5 | Tamamlandı | `make`, `make test`, katı derleyici uyarıları |
| Bellek modeli | %8 | Tamamlandı | Little-endian, hizalama, sınır ve null testleri |
| Decoder ve immediate üretimi | %8 | Tamamlandı | R/I/S/B/U/J alan ve sınır immediate testleri |
| CPU/register/PC ve çalışma döngüsü | %8 | Tamamlandı | CPU durum, fetch, step/run ve x0 testleri |
| ALU ve upper-immediate komutları | %10 | Tamamlandı | Register/immediate ALU ile LUI/AUIPC testleri |
| Branch, JAL ve JALR | %7 | Tamamlandı | Alınan/alınmayan ve hizalama testleri |
| Load/store komutları | %8 | Tamamlandı | Signed/unsigned load, store ve hata atomikliği testleri |
| Hata yönetimi ve uçtan uca ham programlar | %4 | Tamamlandı | Geçersiz kodlama, iki üretilebilir örnek ve otomatik CLI testi |
| Vize belgeleri ve demo paketi | %2 | Tamamlandı | Tasarım, plan, ilerleme, ara rapor ve README |
| **Vize toplam hedefi** | **%60** | **%60 doğrulandı** | `make test`: 32/32; gerçek `.bin` CLI testi başarılı |

## Final hedefi

| İş paketi | Proje ağırlığı | Durum | Kanıt |
| --- | ---: | --- | --- |
| ELF32 yükleyici | %10 | Bekliyor | — |
| Minimal syscall desteği | %5 | Bekliyor | — |
| Etkileşimli debugger | %8 | Bekliyor | — |
| `riscv-tests` ve Spike doğrulaması | %10 | Bekliyor | — |
| Örnek C programları ve performans | %4 | Bekliyor | — |
| Final raporu ve sunum | %3 | Bekliyor | — |
| **Final aşaması toplamı** | **%40** | **Bekliyor** | — |

## Kalite göstergeleri

Uygulama ilerledikçe aşağıdaki alanlar gerçek komut çıktılarıyla güncellenecektir:

- Son temiz derleme tarihi: 2026-11-16
- Derleyici uyarısı: 0
- Toplam test grubu: 32 birim/entegrasyon grubu + CLI regresyon paketi
- Geçen test grubu: 32 + CLI regresyon paketi
- Desteklenen RV32I temel talimatı: 40 (`ECALL` vize aşamasında kontrollü durma)
- Geçen entegrasyon programı sayısı: 3 (iki test programı, bir CLI smoke programı)
- Bilinen çekirdek hatası: 0

## Başlangıç durumu ve risk

`RiscEmu` dizininde başlangıçta yalnızca iki deneysel C programı ve bunların derlenmiş çıktıları bulunmaktadır. Bunlar PC artışı ve bir R tipi komutun alanlarını ayırma denemeleridir; üretim emülatör çekirdeği değildir.

Git çalışma ağacının kökü şu anda `/Users/ismailbakir` olarak algılanmaktadır. Bu durum proje dışındaki çok sayıda dosyanın Git kapsamına girmesine neden olur. Güvenli bir sürüm kontrolü sınırı belirlenmeden commit işlemi yapılmayacaktır.

