# simple_http_server
HARDCORE
Сервер прослушивает порт 2001. Сервер просто в ответ на GET запросы отправляет клиенту html страницы, находящиеся в рабочей директории сервера (исходник, страницы сайта находятся в одной папке, в неё и следует компилировать исходник сервера)
Типичный URL: http://127.0.0.1:2001//main.html
В случае неправильного указания URL, сервер возвращает ошибку 404
