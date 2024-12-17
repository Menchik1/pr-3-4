#include <iostream>
#include <fstream>
#include "json.hpp"
#include <sstream>
#include <filesystem> // Для работы с файловой системой
#include <random>     // Для генерации уникального ключа
#include <vector>     // Для работы с векторами

using namespace std;
using json = nlohmann::json;

namespace fs = std::filesystem;

struct dbase {
    std::string schema_name;
};

std::string generateUniqueKey() {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string key;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, sizeof(alphanum) - 2);

    for (int i = 0; i < 10; ++i) { // Длина ключа 10 символов
        key += alphanum[distribution(generator)];
    }
    return key;
}

// Функция для получения second_lot_id из таблицы пар
int getSecondLotIdFromPair(dbase& db, int pairId) {
    std::string pairFile = db.schema_name + "/pair/1.csv"; // Путь к файлу с парами
    std::ifstream file(pairFile);
    std::string line;

    // Преобразуем pairId в строку
    std::string pairIdStr = std::to_string(pairId);

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string currentPairIdStr, firstLotId, secondLotId;
        std::getline(ss, currentPairIdStr, ',');
        std::getline(ss, firstLotId, ',');
        std::getline(ss, secondLotId, ',');

        // Проверка на корректность данных
        if (currentPairIdStr.empty() || firstLotId.empty() || secondLotId.empty()) {
            continue; // Пропускаем некорректные строки
        }

        // Сравниваем строковые представления
        if (currentPairIdStr == pairIdStr) {
            // Пробуем преобразовать secondLotId в число
            try {
                return std::stoi(secondLotId); // Возвращаем second_lot_id
            } catch (const std::invalid_argument& e) {
                std::cerr << "Ошибка преобразования строки в число: " << e.what() << std::endl;
                return -1; // Возвращаем -1 в случае ошибки
            }
        }
    }
    return -1; // Возвращаем -1, если не найдено
}
int getFirstLotIdFromPair(dbase& db, int pairId) {
    std::string pairFile = db.schema_name + "/pair/1.csv"; // Путь к файлу с парами
    std::ifstream file(pairFile);
    std::string line;

    // Преобразуем pairId в строку
    std::string pairIdStr = std::to_string(pairId);

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string currentPairIdStr, firstLotId, secondLotId;
        std::getline(ss, currentPairIdStr, ',');
        std::getline(ss, firstLotId, ',');
        std::getline(ss, secondLotId, ',');

        // Проверка на корректность данных
        if (currentPairIdStr.empty() || firstLotId.empty() || secondLotId.empty()) {
            continue; // Пропускаем некорректные строки
        }

        // Сравниваем строковые представления
        if (currentPairIdStr == pairIdStr) {
            // Пробуем преобразовать secondLotId в число
            try {
                return std::stoi(firstLotId); // Возвращаем second_lot_id
            } catch (const std::invalid_argument& e) {
                std::cerr << "Ошибка преобразования строки в число: " << e.what() << std::endl;
                return -1; // Возвращаем -1 в случае ошибки
            }
        }
    }
    return -1; // Возвращаем -1, если не найдено
}

// Функция для получения максимального user_id
int getMaxUserId(const std::string& filename) {
    int maxId = 0;
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        std::getline(file, line); // Пропускаем заголовок
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string userIdStr;
            std::getline(ss, userIdStr, ','); // Читаем user_id
            int userId = std::stoi(userIdStr);
            if (userId > maxId) {
                maxId = userId;
            }
        }
        file.close();
    }
    return maxId;
}

// Функция для получения максимального order_id
int getMaxOrderId(const std::string& filename) {
    int maxId = 0;
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        std::getline(file, line); // Пропускаем заголовок
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string orderIdStr;
            std::getline(ss, orderIdStr, ','); // Читаем order_id
            int orderId = std::stoi(orderIdStr);
            if (orderId > maxId) {
                maxId = orderId;
            }
        }
        file.close();
    }
    return maxId;
}

void saveSingleEntryToCSV(dbase& db, const std::string& table, const json& entry) {
    try {
        std::string filename = db.schema_name + "/" + table + "/1.csv"; 
        std::ofstream file(filename, std::ios::app);
        
        // Проверка существования файла для записи заголовков
        if (file) {
            // Запись заголовков только если файл пустой
            if (fs::exists(filename) && fs::file_size(filename) == 0) {
                if (table == "user") {
                    file << "user_id,username,key\n";
                } else if (table == "lot") {
                    file << "lot_id,name\n";
                } else if (table == "user_lot") {
                    file << "user_id,lot_id,quantity\n";
                } else if (table == "order") {
                    file << "order_id,user_id,pair_id,quantity,price,type,closed\n";
                } else if (table == "pair") {
                    file << "pair_id,first_lot_id,second_lot_id\n";
                }
            }

            // Запись данных в файл
            if (table == "user") {
                file << entry["user_id"].get<std::string>() << ","
                     << entry["username"].get<std::string>() << ","
                     << entry["key"].get<std::string>() << "\n";
            } else if (table == "lot") {
                file << entry["lot_id"].get<std::string>() << ","
                     << entry["name"].get<std::string>() << "\n";
            } else if (table == "user_lot") {
                file << entry["user_id"].get<std::string>() << ","
                     << entry["lot_id"].get<std::string>() << ","
                     << entry["quantity"].get<std::string>() << "\n";
            } else if (table == "order") {
                file << entry["order_id"].get<std::string>() << ","
                     << entry["user_id"].get<std::string>() << ","
                     << entry["pair_id"].get<std::string>() << ","
                     << entry["quantity"].get<std::string>() << ","
                     << entry["price"].get<std::string>() << ","
                     << entry["type"].get<std::string>() << ","
                     << entry["closed"].get<std::string>() << "\n";
            } else if (table == "pair") {
                file << entry["pair_id"].get<std::string>() << ","
                     << entry["first_lot_id"].get<std::string>() << ","
                     << entry["second_lot_id"].get<std::string>() << "\n";
            }

        } else {
            throw std::runtime_error("Failed to open data file for saving: " + filename);
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}
// Функция для удаления ордера
void deleteOrder(dbase& db, const std::string& orderId) {
    std::string orderFile = db.schema_name + "/order/1.csv";
    std::ifstream orderStream(orderFile);
    std::string line;
    std::string updatedContent;
    std::string userId;
    double totalCost = 0;
    int pairId = -1;

    // Чтение ордеров и поиск нужного
    while (std::getline(orderStream, line)) {
        std::istringstream ss(line);
        std::string orderIdFromFile, userIdFromFile, pairIdStr, quantityStr, price, type, closed;
        std::getline(ss, orderIdFromFile, ',');
        std::getline(ss, userIdFromFile, ',');
        std::getline(ss, pairIdStr, ',');
        std::getline(ss, quantityStr, ',');
        std::getline(ss, price, ',');
        std::getline(ss, type, ',');
        std::getline(ss, closed, ',');

        if (orderIdFromFile == orderId) {
            userId = userIdFromFile;
            pairId = std::stoi(pairIdStr);
            totalCost = std::stod(quantityStr) * std::stod(price); // Рассчитываем общую стоимость
            std::cout << "Удаление ордера: " << line << std::endl;
            continue; // Пропускаем удаляемый ордер
        }
        updatedContent += line + "\n"; // Сохраняем остальные ордера
    }
    orderStream.close();

    // Запись обновленного содержимого в файл
    std::ofstream outOrderFile(orderFile);
    if (outOrderFile.is_open()) {
        outOrderFile << updatedContent;
        std::cout << "Ордера обновлены и записаны в файл." << std::endl;
    } else {
        std::cout << "Ошибка при открытии файла для записи." << std::endl;
        return;
    }

    // Получаем second_lot_id из пары
    int secondLotId = getSecondLotIdFromPair(db, pairId);
    if (secondLotId == -1) {
        std::cerr << "Ошибка: второй лот не найден для пары ID: " << pairId << std::endl;
        return;
    }

    // Возвращаем количество пользователю
    std::string userLotFile = db.schema_name + "/user_lot/1.csv";
    std::ifstream userLotStream(userLotFile);
    std::string userLotContent;
    std::string updatedUserLotContent;

    // Обновляем количество для пользователя
    while (std::getline(userLotStream, line)) {
        std::istringstream ss(line);
        std::string userIdFromFile, lotId, userQuantityStr;
        std::getline(ss, userIdFromFile, ',');
        std::getline(ss, lotId, ',');
        std::getline(ss, userQuantityStr, ',');

        if (userIdFromFile == userId && lotId == std::to_string(secondLotId)) {
            double currentQuantity = std::stod(userQuantityStr);
            double newQuantity = currentQuantity + totalCost; // Возвращаем полную стоимость
            updatedUserLotContent += userId + "," + lotId + "," + std::to_string(newQuantity) + "\n";
            std::cout << "Обновленный quantity для пользователя " << userId << ": " << newQuantity << std::endl;
        } else {
            updatedUserLotContent += line + "\n"; // Оставляем без изменений
        }
    }
    userLotStream.close();

    // Записываем обновленный баланс обратно в файл
    std::ofstream outUserLotFile(userLotFile);
    if (outUserLotFile.is_open()) {
        outUserLotFile << updatedUserLotContent;
        std::cout << "Баланс пользователя обновлен и записан в файл." << std::endl;
    } else {
        std::cout << "Ошибка при открытии файла для записи." << std::endl;
    }
}



// Функция для добавления нового пользователя
void addUser(dbase& db, const std::string& username, const json& lotData) {
    json newUser;
    
    // Получаем максимальный user_id
    std::string userFile = db.schema_name + "/user/1.csv";
    int maxUserId = getMaxUserId(userFile);
    newUser["user_id"] = std::to_string(maxUserId + 1); // Увеличиваем на 1
    newUser["username"] = username;
    newUser["key"] = generateUniqueKey();

    // Сохранение нового пользователя
    saveSingleEntryToCSV(db, "user", newUser);

    // Пополнение баланса на 1000 единиц каждого доступного лота
    for (const auto& lot : lotData["data"]) {
        json userLotEntry;
        userLotEntry["user_id"] = newUser["user_id"];
        userLotEntry["lot_id"] = lot["lot_id"];
        userLotEntry["quantity"] = "1000"; // Начальный баланс

        saveSingleEntryToCSV(db, "user_lot", userLotEntry);
    }

    std::cout << "Новый пользователь успешно добавлен: " << newUser.dump() << std::endl;
}

// Функция для генерации валютных пар
void generateCurrencyPairs(dbase& db) {
    std::string lotFile = db.schema_name + "/lot/1.csv";
    std::ifstream file(lotFile);
    std::vector<std::string> lots;
    std::string line;

    // Чтение лотов из файла
    std::getline(file, line); // Пропускаем заголовок
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string lotId, lotName;
        std::getline(ss, lotId, ',');
        std::getline(ss, lotName, ',');
        lots.push_back(lotId); // Сохраняем только ID лота
    }
    file.close();

    // Генерация пар
    int pairId = 1;
    for (size_t i = 0; i < lots.size(); ++i) {
        for (size_t j = 0; j < lots.size(); ++j) {
            if (i != j) { // Исключаем одинаковые пары
                json newPair;
                newPair["pair_id"] = std::to_string(pairId++);
                newPair["first_lot_id"] = lots[i]; // Используем ID лота
                newPair["second_lot_id"] = lots[j]; // Используем ID лота
                saveSingleEntryToCSV(db, "pair", newPair);
            }
        }
    }

}

// Функция для создания ордера
void createOrder(dbase& db, const std::string& userId, int pairId, double quantity, double price, const std::string& type) {
    json newOrder;

    // Получаем максимальный order_id
    std::string orderFile = db.schema_name + "/order/1.csv";
    int maxOrderId = getMaxOrderId(orderFile);
    newOrder["order_id"] = std::to_string(maxOrderId + 1);
    newOrder["user_id"] = userId;
    newOrder["pair_id"] = std::to_string(pairId);
    newOrder["quantity"] = std::to_string(quantity);
    newOrder["price"] = std::to_string(price);
    newOrder["type"] = type;
    newOrder["closed"] = "";

    // Сохранение нового ордера
    saveSingleEntryToCSV(db, "order", newOrder);

    // Вычисление стоимости ордера
    double totalCost = quantity * price; // 300 * 0.015 = 4.5
    std::cout << "Создание ордера для пользователя " << userId << " на сумму " << totalCost << std::endl;

    // Получаем second_lot_id из таблицы пар
    int secondLotId = getSecondLotIdFromPair(db, pairId);
    if (secondLotId == -1) {
        std::cerr << "Ошибка: второй лот не найден для пары ID: " << pairId << std::endl;
        return;
    }

    std::string userLotFile = db.schema_name + "/user_lot/1.csv";
    std::ifstream userFile(userLotFile);
    std::string line;
    std::string updatedContent;

    // Проверяем наличие лота для списания
    while (std::getline(userFile, line)) {
        std::istringstream ss(line);
        std::string userIdFromFile, lotId, quantityStr;
        std::getline(ss, userIdFromFile, ',');
        std::getline(ss, lotId, ',');
        std::getline(ss, quantityStr, ',');

        if (userIdFromFile == userId) {
            double currentQuantity = std::stod(quantityStr);

            // Обновляем quantity для второго лота
            if (lotId == std::to_string(secondLotId)) {
                double newQuantity = currentQuantity - totalCost; // Вычитаем quantity
                updatedContent += userId + "," + lotId + "," + std::to_string(newQuantity) + "\n";
                std::cout << "Обновленный quantity: " << newQuantity << std::endl;
            } else {
                updatedContent += line + "\n"; // Оставляем без изменений
            }
        } else {
            updatedContent += line + "\n"; // Оставляем без изменений
        }
    }
    userFile.close();

    // Записываем обновлённый баланс обратно в файл
    std::ofstream outFile(userLotFile);
    if (outFile.is_open()) {
        outFile << updatedContent;
        std::cout << "Баланс обновлен и записан в файл." << std::endl;
    } else {
        std::cout << "Ошибка при открытии файла для записи." << std::endl;
    }
}
void applyOrder(dbase& db, int userId, int orderId) {
    std::string orderFile = db.schema_name + "/order/1.csv";
    std::ifstream orderStream(orderFile);
    std::string line;
    json orderData;

    // Поиск ордера по orderId
    while (std::getline(orderStream, line)) {
        std::istringstream ss(line);
        std::string orderIdFromFile, userIdFromFile, pairIdStr, quantityStr, price, type, closed;
        std::getline(ss, orderIdFromFile, ',');
        std::getline(ss, userIdFromFile, ',');
        std::getline(ss, pairIdStr, ',');
        std::getline(ss, quantityStr, ',');
        std::getline(ss, price, ',');
        std::getline(ss, type, ',');
        std::getline(ss, closed, ',');

        if (orderIdFromFile == std::to_string(orderId)) {
            orderData["order_id"] = orderIdFromFile;
            orderData["user_id"] = userIdFromFile;
            orderData["pair_id"] = pairIdStr;
            orderData["quantity"] = quantityStr;
            orderData["price"] = price;
            orderData["type"] = type;
            break;
        }
    }
    orderStream.close();

    if (orderData.empty()) {
        std::cout << "Ошибка: ордер с ID " << orderId << " не найден." << std::endl;
        return;
    }

    // Получаем second_lot_id из пары
    int pairId = std::stoi(orderData["pair_id"].get<std::string>());
    int secondLotId = getFirstLotIdFromPair(db, pairId);
    double quantity = std::stod(orderData["quantity"].get<std::string>());
    double coefficient = 1.0; // Укажите коэффициент, если он известен

    // Обновление лотов у пользователя, который принимает ордер
    std::string userLotFile = db.schema_name + "/user_lot/1.csv";
    std::ifstream userLotStream(userLotFile);
    std::string updatedUserLotContent;
    double lottosToDeduct = quantity / coefficient;

    // Списываем лоты у пользователя, принимающего ордер
    while (std::getline(userLotStream, line)) {
        std::istringstream ss(line);
        std::string userIdFromFile, lotId, userQuantityStr;
        std::getline(ss, userIdFromFile, ',');
        std::getline(ss, lotId, ',');
        std::getline(ss, userQuantityStr, ',');

        if (userIdFromFile == std::to_string(userId) && lotId == std::to_string(secondLotId)) {
            double currentQuantity = std::stod(userQuantityStr);
            double newQuantity = currentQuantity - lottosToDeduct;

            if (newQuantity < 0) {
                std::cout << "Ошибка: недостаточно лотов для списания." << std::endl;
                return;
            }

            updatedUserLotContent += userIdFromFile + "," + lotId + "," + std::to_string(newQuantity) + "\n";
            std::cout << "Обновленный quantity для пользователя " << userId << ": " << newQuantity << std::endl;
        } else {
            updatedUserLotContent += line + "\n"; // Оставляем без изменений
        }
    }
    userLotStream.close();

    // Записываем обновленный баланс обратно в файл
    std::ofstream outUserLotFile(userLotFile);
    if (outUserLotFile.is_open()) {
        outUserLotFile << updatedUserLotContent;
        std::cout << "Баланс пользователя обновлен и записан в файл." << std::endl;
    } else {
        std::cout << "Ошибка при открытии файла для записи." << std::endl;
    }

    // Теперь добавляем лоты пользователю, чей ордер был принят
    std::string originalUserId = orderData["user_id"].get<std::string>();
    std::string originalUserLotFile = db.schema_name + "/user_lot/1.csv";
    std::ifstream originalUserLotStream(originalUserLotFile);
    std::string originalUpdatedContent;

    while (std::getline(originalUserLotStream, line)) {
        std::istringstream ss(line);
        std::string userIdFromFile, lotId, userQuantityStr;
        std::getline(ss, userIdFromFile, ',');
        std::getline(ss, lotId, ',');
        std::getline(ss, userQuantityStr, ',');

        if (userIdFromFile == originalUserId && lotId == std::to_string(secondLotId)) {
            double currentQuantity = std::stod(userQuantityStr);
            double newQuantity = currentQuantity + lottosToDeduct;

            originalUpdatedContent += originalUserId + "," + std::to_string(secondLotId) + "," + std::to_string(newQuantity) + "\n";
            std::cout << "Обновленный quantity для оригинального пользователя " << originalUserId << ": " << newQuantity << std::endl;
        } else {
            originalUpdatedContent += line + "\n"; // Оставляем без изменений
        }
    }
    originalUserLotStream.close();

    // Записываем обновленный баланс обратно в файл
    std::ofstream outOriginalUserLotFile(originalUserLotFile);
    if (outOriginalUserLotFile.is_open()) {
        outOriginalUserLotFile << originalUpdatedContent;
        std::cout << "Баланс оригинального пользователя обновлен и записан в файл." << std::endl;
    } else {
        std::cout << "Ошибка при открытии файла для записи." << std::endl;
    }

    // Удаляем ордер после его применения
    deleteOrder(db, orderData["order_id"].get<std::string>());
}

void getOrders(dbase& db) {
    std::string orderFile = db.schema_name + "/order/1.csv";
    std::ifstream file(orderFile);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл с ордерами." << std::endl;
        return;
    }

    std::cout << "Список ордеров:" << std::endl;

    // Пропускаем заголовок
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::cout << line << std::endl; // Выводим каждую строку (ордер)
    }

    file.close();
}

void getLots(dbase& db) {
    std::string lotFile = db.schema_name + "/lot/1.csv";
    std::ifstream file(lotFile);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл с лотами." << std::endl;
        return;
    }

    std::cout << "Список лотов:" << std::endl;

    // Пропускаем заголовок
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::cout << line << std::endl; // Выводим каждую строку (лот)
    }

    file.close();
}

void getPairs(dbase& db) {
    std::string pairFile = db.schema_name + "/pair/1.csv";
    std::ifstream file(pairFile);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл с парами." << std::endl;
        return;
    }

    std::cout << "Список пар:" << std::endl;

    // Пропускаем заголовок
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::cout << line << std::endl; // Выводим каждую строку (пара)
    }

    file.close();
}

void getUserAssets(dbase& db, const std::string& userId) {
    std::string userLotFile = db.schema_name + "/user_lot/1.csv";
    std::ifstream file(userLotFile);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл с активами пользователей." << std::endl;
        return;
    }

    std::cout << "Активы пользователя с ID " << userId << ":" << std::endl;

    // Пропускаем заголовок
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string userIdFromFile, lotId, quantityStr;
        std::getline(ss, userIdFromFile, ',');
        std::getline(ss, lotId, ',');
        std::getline(ss, quantityStr, ',');

        if (userIdFromFile == userId) {
            std::cout << "Лот ID: " << lotId << ", Количество: " << quantityStr << std::endl; // Выводим активы
        }
    }

    file.close();
}


int main() {
    dbase db;
    db.schema_name = "Биржа"; // Название папки для хранения данных

    // Создание папки "Биржа", если она не существует
    if (!fs::exists(db.schema_name)) {
        fs::create_directory(db.schema_name);
    }

    // Создание папок для каждой таблицы
    std::string tables[] = {"user", "lot", "user_lot", "order", "pair"};
    for (const auto& table : tables) {
        std::string folder = db.schema_name + "/" + table;
        if (!fs::exists(folder)) {
            fs::create_directory(folder);
        }
    }

    // Открытие файла schema.json
    std::ifstream inputFile("schema.json");
    if (!inputFile.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл schema.json" << std::endl;
        return 1;
    }

    // Парсинг JSON
    json j;
    inputFile >> j;
    inputFile.close();

    // Сохранение данных о пользователях
    for (const auto& user : j["user"]["data"]) {
        saveSingleEntryToCSV(db, "user", user);
    }

    // Сохранение данных о лотах
    for (const auto& lot : j["lot"]["data"]) {
        saveSingleEntryToCSV(db, "lot", lot);
    }

    // Генерация валютных пар
    generateCurrencyPairs(db);

    // Бесконечный цикл для ввода команд
    while (true) {
        std::cout << "Введите команду (или 'exit' для выхода): ";
        std::string command;
        std::getline(std::cin, command);

        if (command == "exit") {
            std::cout << "Выход из программы." << std::endl;
            break; // Выход из бесконечного цикла
        }

        std::istringstream iss(command);
        std::string action;
        iss >> action;

        try {
            if (action == "insert") {
                std::string table;
                iss >> table;

                if (table == "user") {
                    std::string username;
                    iss >> username; // Считываем имя пользователя
                    if (!username.empty()) {
                        json lotData = j["lot"]; 
                        addUser(db, username, lotData);
                    } else {
                        std::cout << "Ошибка: имя пользователя не указано." << std::endl;
                    }
                }
            } else if (action == "assets") {
                std::string userId;
                iss >> userId; // Считываем user_id
                if (!userId.empty()) {
                    getUserAssets(db, userId); // Вызов функции для получения активов пользователя
                } else {
                    std::cout << "Ошибка: user_id не указан." << std::endl;
                }
            } else if (action == "create") {
                std::string type;
                iss >> type;

                if (type == "order") {
                    std::string userId;
                    int pairId;
                    double quantity, price;
                    std::string orderType;

                    iss >> userId >> pairId >> quantity >> price >> orderType;
                    createOrder(db, userId, pairId, quantity, price, orderType);
                }
            } else if (action == "list") {
            std::string type;
            iss >> type;

            if (type == "orders") {
                getOrders(db); // Вызов функции для получения списка ордеров
            } else if (type == "lots") {
                getLots(db); // Вызов функции для получения списка лотов
            } else if (type == "pairs") {
                getPairs(db); // Вызов функции для получения списка пар
            }
            }else if (action == "apply") {
                int userId, orderId;
                iss >> userId >> orderId; // Считываем user_id и order_id
                applyOrder(db, userId, orderId);
            } else if (action == "delete") {
                std::string type;
                iss >> type;

                if (type == "order") {
                    std::string orderId;
                    iss >> orderId; // Считываем order_id
                    if (!orderId.empty()) {
                        deleteOrder(db, orderId);
                    } else {
                        std::cout << "Ошибка: order_id не указано." << std::endl;
                    }
                }
            } else {
                throw runtime_error("Неизвестная команда: " + command);
            }
        } catch (const exception& e) {
            std::cout << "Ошибка: " << e.what() << std::endl;
        }
    }

    return 0;
}
