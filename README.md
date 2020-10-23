![PROJECT_PHOTO](https://github.com/VICLER/GyverDrink/blob/master/proj_img.jpg)
# Автоматический наливатор напитков на Arduino
* [Описание проекта](#chapter-0)
* [Схемы подключения](#chapter-1)
* [Полезная информация](#chapter-2)
* [Поддержка](#chapter-3)

<a id="chapter-0"></a>
## Описание проекта
Модифицированная версия прошивки к проекту "Наливатор by AlexGyver"
- Система рассчитана на 1-6 рюмок
- Динамическая подсветка рюмок
- Любые виды датчиков наличия рюмок (механические, ёмкостные, ик)
- Пищевая мембранная помпа
- Энкодер для управления, дисплей для отображения информации о режиме, обьёме
- Доступны семисегментный дисплей на контроллере TM1637 и OLED на контроллерах SSD1306 (0.96"), SH1106 (1.3"), SSD1309 (1.54") или  (2.42")
- Возможность подключения шагового двигателя для более плавных и бесшумных движений
- Работа устройства как от внешнего источника питания, так и от встроенного аккумулятора с мониторингом напряжения
- Полная калибровка устройства в сервис режиме без необходимости перепрошивки (после выхода из сервис режима данные сохраняются в памяти)
- Полноценное меню с настройками, калибровкой и статистикой в OLED версии прошивки
- Страница исходного проекта на сайте: https://alexgyver.ru/GyverDrink/

[Демонстрация версии с OLED дисплеем](https://youtu.be/Gz3xwlsXfk0)

<a id="chapter-1"></a>
## Схемы
- [Схема исходного проекта](https://github.com/AlexGyver/GyverDrink) *(все настройки подключений для каждой прошивки так же находятся в Config.h в секции "Соедидения")*
![SCHEME](https://github.com/VICLER/GyverDrink/blob/master/schemes/scheme1.jpg)
![SCHEME](https://github.com/VICLER/GyverDrink/blob/master/schemes/scheme2.jpg)
- Схема модификации сенсорной клавиатуры *(по желанию - подключается вместо кнопок/датчиков)*
![SCHEME](https://github.com/VICLER/GyverDrink/blob/master/schemes/TTP229_configuration.jpg)
- Схема модификации шагового двигателя для подключения *(по желанию - подключается вместо сервопривода)*
![SCHEME](https://github.com/VICLER/GyverDrink/blob/master/schemes/StepMot_connection.jpg)

<a id="chapter-2"></a>
## Полезная информация
* [Сайт AlexGyver](http://alexgyver.ru/)
* [Обсуждение проекта на форуме](https://community.alexgyver.ru/threads/nalivator-obsuzhdenie-proekta.1880/) *(Вопросы задавать тут)*
* [Основной YouTube канал AlexGyver](https://www.youtube.com/channel/UCgtAOyEQdAyjvm9ATCi_Aig?sub_confirmation=1)
* [YouTube канал про Arduino от AlexGyver](https://www.youtube.com/channel/UC4axiS76D784-ofoTdo5zOA?sub_confirmation=1)
* [Видеоуроки по пайке](https://www.youtube.com/playlist?list=PLOT_HeyBraBuMIwfSYu7kCKXxQGsUKcqR)
* [Видеоуроки по Arduino](http://alexgyver.ru/arduino_lessons/)

<a id="chapter-3"></a>
## Поддержка
* [PayPal](http://paypal.me/vicler) *(viktorglekler@googlemail.com)
