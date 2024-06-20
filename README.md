### Сборка

Перед тем, как начать сборку необходимо подтянуть самбодуль для работы с json:
```
git submodule init
git submodule update
```

Сборка проекта:
```
mkdir build && cd build
cmake ..
make
```

### Запуск

Можно передавать параметры программы через переменные окружения:
```
export TARGET_DIR_PATH=<path to target directory>
export PERIODE=<check sum periode in sec>
```

Если переменные окружения не установлены, можно передать их через параметры запуска программы (см. --help):
```
./main --directory <path to target directory> --periode <check sum periode in sec>
```

### Остановка
Отсановка программы происходит по сигналу SIGTERM.

### Запуск тестов
В проекте предусмотрены pytests. Для их запуска предварительно соберите саму программу, а затем запускайте тесты
```
pytest simple_tests.py
```

Также, отдельным файлом представлен стресс тест, его работа может занять большое количество времени:
```
pytest stress_test.py
```