![PROJECT_PHOTO](https://github.com/VICLER/GyverDrink/blob/master/proj_img.jpg)
# Автоматический наливатор напитков на Arduino
* [Описание проекта](#chapter-0)
* [Папки проекта](#chapter-1)
* [Схемы подключения](#chapter-2)
* [Материалы и компоненты](#chapter-3)
* [Как скачать и прошить](#chapter-4)
* [FAQ](#chapter-5)
* [Полезная информация](#chapter-6)
* [Поддержка](#chapter-7)

<a id="chapter-0"></a>
## Описание проекта
Модифицированная версия прошивки к проекту "Наливатор by AlexGyver"
- Система рассчитана на 1-6 рюмок
- Динамическая подсветка рюмок
- Любые виды датчиков наличия рюмок (механические, ёмкостные, ик)
- Пищевая мембранная помпа
- Энкодер для управления, дисплей для отображения информации о режиме, обьёме
- Доступны семисегментный дисплей на контроллере TM1637 и OLED на контроллерах SSD1306 (0.96") или SH1106 (1.3")
- Возможность подключения шагового двигателя для более плавных и бесшумных движений
- Работа устройства как от внешнего источника питания, так и от встроенного аккумулятора с мониторингом напряжения
- Полная калибровка устройства в сервис режиме без необходимости перепрошивки (после выхода из сервис режима данные сохраняются в памяти)
- Полноценное меню с настройками, калибровкой и статистикой в OLED версии прошивки
- Страница исходного проекта на сайте: https://alexgyver.ru/GyverDrink/

<a id="chapter-1"></a>
## Папки
**ВНИМАНИЕ! Если это твой первый опыт работы с Arduino, читай [инструкцию](#chapter-4)**
- **libraries** - библиотеки проекта. Заменить имеющиеся версии
- **firmware** - прошивки для Arduino
- **schemes** - схемы подключения компонентов

<a id="chapter-2"></a>
## Схемы
![SCHEME](https://github.com/AlexGyver/GyverDrink/blob/master/schemes/scheme1.jpg)
![SCHEME](https://github.com/AlexGyver/GyverDrink/blob/master/schemes/scheme2.jpg)
![SCHEME](https://github.com/VICLER/GyverDrink/blob/master/schemes/TTP229_configuration.jpg)
![SCHEME](https://github.com/VICLER/GyverDrink/blob/master/schemes/StepMot_connection.jpg)

<a id="chapter-3"></a>
## Материалы и компоненты
### Ссылки оставлены на магазины, с которых я закупаюсь уже не один год
- Arduino NANO https://ali.ski/allgN
- Адресная лента (под нарезку) https://ali.ski/xIkXAq
- Адресные модули поштучно https://ali.ski/cGc-uI
- Энкодер https://ali.ski/KZTbG
- Кнопка https://ali.ski/GRWZU
- Дисплей https://ali.ski/-lC8C
- Концевики https://ali.ski/f9qTis
- Драйвер https://ali.ski/pjXPVR
- Серво https://ali.ski/iWDe1i
- Модуль USB https://ali.ski/5qKhEX
- Помпа https://ali.ski/MU0lo  https://ali.ski/C_LOCk
- Шаговый двигатель https://aliexpi.com/UvLc
- Повышающий преобразователь https://aliexpi.com/vQ6O
- Модуль заряда Li аккумулятора https://aliexpi.com/mlG4


## Вам скорее всего пригодится
* [Всё для пайки (паяльники и примочки)](http://alexgyver.ru/all-for-soldering/)
* [Недорогие инструменты](http://alexgyver.ru/my_instruments/)
* [Все существующие модули и сенсоры Arduino](http://alexgyver.ru/arduino_shop/)
* [Электронные компоненты](http://alexgyver.ru/electronics/)
* [Аккумуляторы и зарядные модули](http://alexgyver.ru/18650/)

<a id="chapter-4"></a>
## Как скачать и прошить
* [Первые шаги с Arduino](http://alexgyver.ru/arduino-first/) - ультра подробная статья по началу работы с Ардуино, ознакомиться первым делом!
* Скачать архив с проектом
> На главной странице проекта (где ты читаешь этот текст) вверху справа зелёная кнопка **Clone or download**, вот её жми, там будет **Download ZIP**
* Установить библиотеки в  
`C:\Program Files (x86)\Arduino\libraries\` (Windows x64)  
`C:\Program Files\Arduino\libraries\` (Windows x86)
* **Подключить внешнее питание 5 Вольт**
* Подключить плату к компьютеру
* Файл/Настройки, вставить ссылку http://arduino.esp8266.com/stable/package_esp8266com_index.json в в текст-бокс для дополнительных ссылок для менеджера плат
* Открыть Инструменты/Плата/Менеджер плат…, найти esp8266 by ESP8266 Community, установить версию 2.5.0
* Открыть Инструменты, настроить Плату, Порт и другие параметры как на скриншоте
* Загрузить прошивку

## Настройки в коде

	
<a id="chapter-5"></a>
## FAQ
### Основные вопросы
В: Как скачать с этого грёбаного сайта?  
О: На главной странице проекта (где ты читаешь этот текст) вверху справа зелёная кнопка **Clone or download**, вот её жми, там будет **Download ZIP**

В: Скачался какой то файл .zip, куда его теперь?  
О: Это архив. Можно открыть стандартными средствами Windows, но думаю у всех на компьютере установлен WinRAR, архив нужно правой кнопкой и извлечь.

В: Я совсем новичок! Что мне делать с Ардуиной, где взять все программы?  
О: Читай и смотри видос http://alexgyver.ru/arduino-first/

В: Вылетает ошибка загрузки / компиляции!
О: Читай тут: https://alexgyver.ru/arduino-first/#step-5

В: Сколько стоит?  
О: Ничего не продаю.

<a id="chapter-6"></a>
## Полезная информация
* [Сайт AlexGyver](http://alexgyver.ru/)
* [Основной YouTube канал AlexGyver](https://www.youtube.com/channel/UCgtAOyEQdAyjvm9ATCi_Aig?sub_confirmation=1)
* [YouTube канал про Arduino от AlexGyver](https://www.youtube.com/channel/UC4axiS76D784-ofoTdo5zOA?sub_confirmation=1)
* [Видеоуроки по пайке](https://www.youtube.com/playlist?list=PLOT_HeyBraBuMIwfSYu7kCKXxQGsUKcqR)
* [Видеоуроки по Arduino](http://alexgyver.ru/arduino_lessons/)

<a id="chapter-7"></a>
## Поддержка
* [PayPal](http://paypal.me/vicler)
