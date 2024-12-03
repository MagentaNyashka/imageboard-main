# ~~TCP чат by CyberConnect~~ Доработанная версия сервера для привязки к СУБД PSQL

Здравствуйnе, ~~мы участники хакатона от группы «Астра»~~ я разработчик, решившийся переписать логику для сервера для лучшей поддерживаемости и модульности. Данная версия была создана для проекта по Разработке Программных Модулей, темой которого стал "WPF-Мессенджер".

## Содержание
- [Технологии](#технологии)
- [Начало работы](#начало-работы)
- [To do](#to-do)
- [Команда проекта](#команда-проекта)
- [Источники](#Источники)

## Технологии
- [postgresql](https://www.postgresql.org/)
- [pqxx](https://github.com/jtv/libpqxx)
- [nlohmann/json](https://github.com/nlohmann/json)
- ~~[boost/websocket](https://github.com/boostorg)~~
- [sys/socket]

## Начало-работы
Команды приведены для ~~OS astra linux~~ любого дистрибутива Linux на котором вы сможете настроить зависимости

### Установка

~~Библиотеку boost для работы на OS astra linux нужно скомпилировать [самостоятельно](https://stackoverflow.com/questions/12578499/how-to-install-boost-on-ubuntu)~~
Это больше не WebSocket это обыкновенный TCP Socket

Выполните сборку:
```sh
cd imageboard-main
mkdir build && cd build
cmake ..
cmake --build .
```

### Зачем ~~мы~~ я разработал этот проект?
Я додик

## Команда проекта

- ~~[Артём Шакиров](https://github.com/artemka-sh) — Beckend Engineer~~
- ~~[Максим Журавлёв](https://github.com/makszhuravlev) — Front-End Engineer~~
- ~~[Александр Степанцов](https://github.com/sashok112) — Team Leader~~
- [Леонид Лопатин](https://github.com/MagentaNyashka) — Я ОДИН

## Источники
[Source](https://ru.wikipedia.org/wiki/%D0%A8%D0%B8%D0%B7%D0%BE%D1%84%D1%80%D0%B5%D0%BD%D0%B8%D1%8F)
