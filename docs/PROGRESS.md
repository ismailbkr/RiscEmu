# RV32I Emülatörü — İlerleme Takibi

## Değerlendirme yöntemi

İlerleme yüzdesi satır sayısına göre değil, doğrulanmış iş paketlerinin ağırlığına göre hesaplanır. Bir kalem ancak uygulama, test ve belge kanıtı birlikte mevcutsa tamamlanır.

## Vize hedefi

| İş paketi | Proje ağırlığı | Durum | Kanıt |
| --- | ---: | --- | --- |
| Proje yapısı, Makefile ve test altyapısı | %5 | Tamamlandı | `make`, `make test`, katı derleyici uyarıları |
| Bellek modeli | %8 | Tamamlandı | Little-endian, hizalama, sınır ve null testleri |
| Decoder ve immediate üretimi | %8 | Tamamlandı | R/I/S/B/U/J alan ve sınır immediate testleri |
| CPU/register/PC ve çalışma döngüsü | %8 | Planlandı | — |
| ALU ve upper-immediate komutları | %10 | Planlandı | — |
| Branch, JAL ve JALR | %7 | Planlandı | — |
| Load/store komutları | %8 | Planlandı | — |
| Hata yönetimi ve uçtan uca ham programlar | %4 | Planlandı | — |
| Vize belgeleri ve demo paketi | %2 | Planlandı | Bu dokümanların ilk sürümü |
| **Vize toplam hedefi** | **%60** | **%21 doğrulandı** | Aşama 0–2 kalite kapıları geçti |

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
- Toplam test sayısı: 12 (Aşama 0–2)
- Geçen test sayısı: 12 (Aşama 0–2)
- Desteklenen RV32I komut sayısı: —
- Geçen entegrasyon programı sayısı: —
- Bilinen hata sayısı: —

## Başlangıç durumu ve risk

`RiscEmu` dizininde başlangıçta yalnızca iki deneysel C programı ve bunların derlenmiş çıktıları bulunmaktadır. Bunlar PC artışı ve bir R tipi komutun alanlarını ayırma denemeleridir; üretim emülatör çekirdeği değildir.

Git çalışma ağacının kökü şu anda `/Users/ismailbakir` olarak algılanmaktadır. Bu durum proje dışındaki çok sayıda dosyanın Git kapsamına girmesine neden olur. Güvenli bir sürüm kontrolü sınırı belirlenmeden commit işlemi yapılmayacaktır.

