# OpenHXServer

OpenHXServer или просто OpenHX - открытая реализация протокола HX, разработанного участником форума zx-pk.ru под ником Patron.

Протокол позволяет загружать ОС RT-11 на компьютерах [УКНЦ](https://ru.wikipedia.org/wiki/%D0%AD%D0%BB%D0%B5%D0%BA%D1%82%D1%80%D0%BE%D0%BD%D0%B8%D0%BA%D0%B0_%D0%9C%D0%A1_0511) (и другие, например ДВК) через Стык С2 (COM порт) с образов дискет, находящися на современном компьютере.

Подробнее - [тут](https://zx-pk.ru/threads/20683-protokol-hx-imitatsiya-blochnogo-ustrojstva-s-posledovatelnym-interfejsom.html).

Кратко - драйвер HX.SYS должен быть записан на образ дискеты .dsk, который "монтируется" в слот HX0 в программе.

# Смежные проекты

[Эмулятор УКНЦ by Nikita Zimin](https://github.com/nzeemin/ukncbtl/) - тоже может загружаться по протоколу HX (нужно настроить вирутальные ком-порты, например программой com0com)

[Утилиты by Nikita Zimin](https://github.com/nzeemin/ukncbtl-utils/) - полезные утилиты для редактирования образов и др.


Пока не реализован терминал. 

[Сборка для Windows](https://disk.yandex.ru/d/V_BeXg2tUqwDPw)

[Сборка для Windows XP](https://disk.yandex.ru/d/lZ2vvSD9tmcCIw)

[Сборка для Linux (.deb)](https://disk.yandex.ru/d/Gig7HQGHeMXBCw)


![alt text](screenshots/dsk_switching.png "Загрузка RT-11.")

## Лицензия
[BSD](https://choosealicense.com/licenses/bsd-2-clause/)
