🧠 Uçta Yapay Zeka (Edge AI): Baremetal STM32 Üzerinde MNIST El Yazısı Tanıma
STM32PythonOpenCVBaremetal

Bu proje, bilgisayar kamerasından alınan el yazısı rakamların bilgisayarlı görü (Computer Vision) algoritmalarıyla işlenip UART üzerinden STM32F439ZI mikrodenetleyicisine aktarıldığı ve mikrodenetleyici içine gömülmüş optimize bir Yapay Sinir Ağı (X-CUBE-AI) tarafından gerçek zamanlı olarak sınıflandırıldığı kapsamlı bir uçta yapay zeka (Edge AI) sistemidir.

Projenin en büyük mühendislik özelliği; yapay zeka modelini mikrodenetleyicide koştururken üretici firmanın hantal Donanım Soyutlama Katmanı (HAL) kütüphanelerini kullanmak yerine, donanım kaynaklarını ve zamanlamayı optimize etmek amacıyla tamamen Baremetal (Yazmaç/Register Seviyesi) C programlama felsefesiyle geliştirilmiş olmasıdır.

🚀 HIZLI BAŞLANGIÇ NOTU: Bu doküman, projenin yapay zeka, görüntü işleme ve donanım mimarisini (baremetal) anlatan çok detaylı teknik bir mühendislik raporudur. Teorik detayları atlayıp projeyi doğrudan bilgisayarınızda çalıştırmak ve denemek istiyorsanız, sayfanın en altındaki "6. Kurulum ve Çalıştırma Adımları" bölümüne atlayabilirsiniz.

📖 1. Projenin Amacı ve Özeti
Günümüzde yapay zeka modelleri genellikle devasa sunucularda (Cloud) çalıştırılır. Ancak savunma sanayii, IoT, otonom araçlar ve kapalı devre sistemlerde verinin buluta gidip gelmesi gecikme (latency), güvenlik açıkları ve yüksek güç tüketimi yaratır.

Amaç: Bu proje, derin öğrenme algoritmalarının buluta ihtiyaç duymadan, kilobaytlar seviyesinde belleğe sahip bir mikrodenetleyicide (Edge Device) nasıl bağımsız çalıştırılabileceğini kanıtlamayı amaçlar. Özetle sistem şu döngüyü icra eder:

Python betiği web kamerasından aldığı el yazısını MNIST standardına dönüştürür.
UART üzerinden STM32'ye iletir.
STM32 içindeki gömülü model (hiçbir işletim sistemi olmadan) çıplak donanım üzerinde tahminde bulunur.
Sonuçlar opsiyonel bir LCD ekrana ve fiziksel LED'lere yansıtılır.
🛠️ 2. Kullanılan Donanımlar ve Geliştirme Ortamı
Mikrodenetleyici (Ana Geliştirme Kartı): STM32 Nucleo-144 (NUCLEO-F439ZI)
Mimarî: ARM Cortex-M4 (180 MHz) - Donanımsal FPU (Kayan Nokta Ünitesi) destekli.
Hafıza: 2 MB Flash (ROM) ve 256 KB SRAM. (Bu yüksek kapasite, yapay zeka modelinin ağırlıklarını Flash'a gömmek ve katmanlar arası tensör hesaplamalarını RAM'de rahatça yapmak için kritik bir avantaj sağlamıştır.)
Kamera: Standart PC Web Kamerası (Python üzerinden görüntü yakalamak için).
Gösterge (Opsiyonel): 16x2 Karakter LCD ve PCF8574 I2C Genişletici modülü. (Eğer elinizde LCD yoksa, sonuçları sadece bilgisayar terminalinden ve kart üzerindeki LED'lerden de görebilirsiniz).
Yazılım Ortamı:
STM32CubeIDE (C/C++ Derleyici ve IDE)
STM32CubeMX & X-CUBE-AI Eklentisi (Model dönüştürme için)
Python 3.x (opencv-python, pyserial, numpy)
⚠️ Kritik Uyarı (Sürüm Uyumluluğu): Projeyi kendi bilgisayarınızda derlerken STM32CubeMX, STM32CubeIDE ve X-CUBE-AI paketlerinin mutlaka en güncel sürümlerinde olduğundan emin olun. STMicroelectronics, AI motorunun (CMSIS-NN) çekirdek dosyalarını sürekli günceller. Eski bir CubeMX sürümü kullanmak, C koduna çevrilen yapay zeka kütüphanelerinde sürüm çakışmalarına (Build Error) neden olabilir.

🧠 3. ST ModelZoo ve STM32Cube.AI (Yapay Zeka Hazırlığı)
Bilgisayarda Python ile yazılmış devasa bir .h5 veya .tflite modelini mikrodenetleyiciye doğrudan atamazsınız. Bu projede modelin donanıma uyarlanması iki aşamada gerçekleşmiştir:

Adım 3.1: ModelZoo'dan Optimize Modelin Alınması (st_mnistv1.tflite)
Projede sıfırdan bir model eğitilmemiş, bunun yerine STMicroelectronics'in GitHub üzerindeki stm32ai-modelzoo deposunda gömülü sistemler için özel olarak sunduğu st_mnistv1 modeli kullanılmıştır.

Neden Bu Model? Bu model rastgele bir Evrişimli Sinir Ağı (CNN) değildir. Özel olarak Cortex-M işlemciler için optimize edilmiştir. Katman sayıları azaltılmış (Topology Optimization), gereksiz ağırlıklar budanmış (Pruning) ve düşük RAM/Flash tüketimi için tasarlanmıştır. Model, X-CUBE-AI motoruna TensorFlow Lite formatında (.tflite) beslenmiştir.

Adım 3.2: STM32Cube.AI / AI Studio Üzerinde Neler Yaptık?
Optimize modeli CubeMX içindeki X-CUBE-AI arayüzüne yükledikten sonra şu işlemler yapılmıştır:

Analiz (Analyze): Modelin mikrodenetleyici üzerinde çalışırken kaç KB Flash (ROM) ve kaç KB RAM harcayacağı analiz edilmiştir.
Bellek Optimizasyonu (Memory Reuse): RAM tasarrufu için katmanlar arası geçici tensörlerin aynı bellek alanını üzerine yazarak (Ping-Pong buffer) kullanması aktif edilmiştir.
C Koduna Çevrim: Modelin ağırlıkları (weights) onaltılık const C dizilerine dönüştürülmüş ve ARM CMSIS-NN kütüphanesi kullanılarak son derece hızlı matris çarpım fonksiyonları üretilmiştir.
🐍 4. Python Görüntü İşleme Pipeline'ı (ai_camera.py)
Kameradan alınan bir görüntüyü yapay zekaya doğrudan verirseniz çalışmaz. Model, tam olarak eğitildiği formattaki (MNIST) veriyi bekler. Python tarafında şu işlemler sırasıyla uygulanır:

UI Kirliliğini Önleme: Ekrana çizilen yeşil ROI (Region of Interest) karesinin yapay zekayı bozmaması için görüntü önce clean_frame.copy() ile temiz bir şekilde kopyalanır.
Bulanıklaştırma ve Gri Tonlama: Renkler silinir ve sensör kirliliğini (noise) yok etmek için Gauss Filtresi (GaussianBlur) uygulanır.
Adaptif Eşikleme (Adaptive Threshold): Odanın ışığı kağıdın her yerine eşit vurmayabilir. Sabit bir eşik (Otsu) yerine cv2.adaptiveThreshold kullanılarak, gölgeler filtrelenir. Arka plan tamamen siyah (0), kalem izi beyaz (255) yapılır.
Morfolojik Operasyonlar (Morphology): Kalem izlerindeki el titremesinden kaynaklı "kopuklukları" lehimlemek/birleştirmek için görüntünün beyaz pikselleri cv2.MORPH_CLOSE ile genleştirilir ve tekrar aşındırılır.
Boyutlandırma ve Merkeze Alma (Padding): Rakamın dış hatları bulunur (Bounding Box), en-boy oranı bozulmadan 20x20 boyutuna küçültülür ve MNIST standardı olan 28x28 siyah bir tuvalin tam merkezine oturtulur.
Seri Aktarım: Elde edilen 784 bayt, 115200 Baud ile tek paket halinde STM32'ye yollanır.
⚙️ 5. Gömülü Yazılım ve main.c Detayları (Baremetal)
ST'nin otomatik ürettiği C kodlarındaki tüm HAL kütüphaneleri (Hardware Abstraction Layer) projeden tamamen silinmiştir. Çevre birimleri doğrudan işlemci yazmaçları (Registers) üzerinden sürülmüştür.

main.c dosyasında sırasıyla şu hayati donanım operasyonları icra edilir:

Kriz 1: FPU (Kayan Nokta Ünitesi) Aktivasyonu
Yapay zeka Float32 (ondalıklı) matris çarpımı yapar. Ancak Cortex-M4'te FPU donanımı varsayılan olarak enerji tasarrufu için kapalıdır. Model çalıştığı an işlemci HardFault (Donanım Çökmesi) verir. Bunu aşmak için main.c başına şu baremetal komut eklenerek FPU zorla uyandırılmıştır:

c

(*(volatile uint32_t *)0xE000ED88) |= ((3UL << 10*2) | (3UL << 11*2));
Kriz 2: Preallocated Bellek Bağlama
Model, parçalanmayı önlemek için RAM'de malloc() kullanmaz (STAI_FLAG_PREALLOCATED). Bizim ona statik bir bellek adresi vermemiz gerekir. main.c üzerinden statik bellek dizileri oluşturulmuş ve modele köprülenmiştir:

c

stai_network_set_inputs(network, &in_data);
stai_network_set_outputs(network, &out_data);
Kriz 3: Sürekli '8' Tahmini ve Normalizasyon Matematiği
Python'dan 0 (siyah) ve 255 (beyaz) olarak gelen pikseller modele doğrudan verilirse model sürekli '8' tahmin eder. Çünkü ST'nin modeli [-1.0, 1.0] aralığında eğitilmiştir. Pikseller main.c içinde donanımsal olarak şu şekilde normalize edilir (Siyah = -1.0, Beyaz = 1.0):

c

for(int i=0; i<784; i++) {
    ai_in[i] = ((float)img_data[i] / 127.5f) - 1.0f; 
}
Sonuç Tespiti (ArgMax Algoritması)
Model çıkışında oluşan 36 sınıflık (0-9 ve A-Z) olasılık dizisi C dilinde for döngüsüyle taranır (ArgMax). En yüksek olasılığa sahip indeks tespit edilir. Bulunan sonuç I2C protokolü üzerinden (isteğe bağlı) LCD ekrana yazdırılır ve karta entegre LED, tespit edilen rakam sayısı kadar yanıp sönerek (Örn: Sonuç 3 ise 3 kez göz kırpar) fiziksel geri bildirim verir.

🚀 6. Kurulum ve Çalıştırma Adımları
Donanım Bağlantıları:

STM32 Nucleo kartınızı USB ile bilgisayara bağlayın.
(Opsiyonel) LCD Ekran için: PCF8574 SCL pinini PB8'e, SDA pinini PB9'a bağlayın.
Gömülü Yazılımın Yüklenmesi:

Bu depoyu klonlayın ve STM32CubeIDE ile projeyi açın. (HAL kütüphanesi olmadığı için derleme sadece birkaç saniye sürer). Kodunuzu karta "Run" (veya Debug) seçeneğiyle Flash'layın.
Python Ortamı ve Bağımlılıkların Kurulumu:

Bilgisayarınızda Python 3.8 veya üzeri bir sürümün yüklü olduğundan emin olun.
Başlat menüsünden cmd (Komut İstemcisi), PowerShell veya VS Code terminalinizi açın.
Projenin görüntü işleme ve seri port haberleşme fonksiyonlarını yürütebilmesi için aşağıdaki komutu kopyalayıp terminale yapıştırarak gerekli kütüphaneleri indirin:
bash

pip install opencv-python pyserial numpy
COM Portunun Bulunması ve Python Koduna Entegre Edilmesi:

STM32 kartınız bilgisayara USB ile bağlıyken, Windows'ta Aygıt Yöneticisi'ni (Device Manager) açın.
Listeden "Bağlantı Noktaları (COM ve LPT)" sekmesini genişletin.
Orada STMicroelectronics Sanal Seri Portunun hangi numarayı aldığını not edin (Örneğin: COM3, COM7, COM12 vb.).
Proje klasöründeki ai_camera.py dosyasını bir metin editörüyle (VS Code, Notepad++ veya IDLE) açın.
Dosyanın hemen başlarındaki ayarlar bloğunda yer alan COM_PORT değişkenini bulun ve kendi port numaranızla değiştirin. (Ayrıca bilgisayarınızda birden fazla kamera varsa ve programı açtığınızda siyah ekran alırsanız CAMERA_ID değerini 1 veya 2 yaparak harici kameraya geçebilirsiniz):
python

# --- AYARLAR (SETTINGS) ---
COM_PORT = 'COM12'   # <-- Aygıt Yöneticisinden bulduğunuz port numarasını buraya yazın
BAUD_RATE = 115200
CAMERA_ID = 0        # <-- Birden fazla kamera varsa 1, 2 olarak değiştirin
Test Aşaması:

python ai_camera.py komutunu çalıştırın.
Kamera penceresi açılacaktır. Beyaz bir kağıda siyah ve kalın bir kalemle rakam çizin.
Rakamı yeşil karenin içine ortalayın ve klavyeden s (Send) tuşuna basın.
Terminalde ve (varsa) LCD ekranda saniyenin onda biri sürede tahmin sonucunu göreceksiniz!
🛠️ 7. Geliştiriciler İçin: Bu Projeye Neler Eklenebilir?
Bu proje donanım seviyesinde tamamen şeffaf ve açık uçlu tasarlanmıştır. Eğer projeyi daha da ileri taşımak isterseniz aşağıdaki modülleri projeye entegre edebilirsiniz:

DMA (Direct Memory Access) Entegrasyonu: Şu an UART'tan veri bayt bayt polling yöntemiyle alınmaktadır. İşlemciyi tamamen boşa çıkarmak için UART alımını DMA kanallarına yönlendirebilirsiniz.
INT8 Kuantizasyonu: Cube.AI üzerinden modeli INT8 formatında yeniden derleyerek RAM tüketimini 4 kat daha azaltıp çıkarım süresini hızlandırabilirsiniz.
Fiziksel Kamera Modülü (Stand-alone Sistem): Bilgisayardan veri yollamak yerine, STM32'nin DCMI arayüzüne doğrudan bir OV7670 kamera modülü bağlayarak sistemi bilgisayardan tamamen bağımsız (Otonom Edge AI) hale getirebilirsiniz.
Bu açık kaynaklı proje, Uçta Yapay Zeka (Edge AI) ve Baremetal Gömülü Sistem mühendisliği konularına ilgi duyan tüm geliştiricilerin kullanımına ve katkısına açıktır.
