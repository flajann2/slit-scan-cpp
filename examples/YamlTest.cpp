#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <yaml-cpp/yaml.h>

/// struct InventoryItem {
///     std::string name;
///     int quantity;
/// 
///     // Function to serialize InventoryItem to YAML
///     YAML::Node toYAML() const {
///         YAML::Node node;
///         node["name"] = name;
///         node["quantity"] = quantity;
///         return node;
///     }
/// 
///     // Function to deserialize InventoryItem from YAML
///     static InventoryItem fromYAML(const YAML::Node& node) {
///         InventoryItem item;
///         item.name = node["name"].as<std::string>();
///         item.quantity = node["quantity"].as<int>();
///         return item;
///     }
/// };
/// 
/// struct Inventory {
///     std::vector<InventoryItem> items;
///     std::map<std::string, std::string> attributes;
/// 
///     // Function to serialize Inventory to YAML
///     YAML::Node toYAML() const {
///         YAML::Node node;
///         
///         // Serialize vector of items
///         for (const auto& item : items) {
///             node["items"].push_back(item.toYAML());
///         }
/// 
///         // Serialize map of attributes
///         for (const auto& [key, value] : attributes) {
///             node["attributes"][key] = value;
///         }
/// 
///         return node;
///     }
/// 
///     // Function to deserialize Inventory from YAML
///     static Inventory fromYAML(const YAML::Node& node) {
///         Inventory inventory;
/// 
///         // Deserialize vector of items
///         for (const auto& itemNode : node["items"]) {
///             inventory.items.push_back(InventoryItem::fromYAML(itemNode));
///         }
/// 
///         // Deserialize map of attributes
///         for (const auto& attr : node["attributes"]) {
///             inventory.attributes[attr.first.as<std::string>()] = attr.second.as<std::string>();
///         }
/// 
///         return inventory;
///     }
/// };

struct Color {
    int r, g, b;

    // Convert Color to/from YAML
    YAML::Node toYAML() const {
        YAML::Node node;
        node["r"] = r;
        node["g"] = g;
        node["b"] = b;
        return node;
    }

    static Color fromYAML(const YAML::Node& node) {
        return {node["r"].as<int>(), node["g"].as<int>(), node["b"].as<int>()};
    }
};

struct Stage {
    int distance;
    Color color1;
    Color color2;

    // Convert Stage to/from YAML
    YAML::Node toYAML() const {
        YAML::Node node;
        node["distance"] = distance;
        node["color1"] = color1.toYAML();
        node["color2"] = color2.toYAML();
        return node;
    }

    static Stage fromYAML(const YAML::Node& node) {
        Stage stage;
        stage.distance = node["distance"].as<int>();
        stage.color1 = Color::fromYAML(node["color1"]);
        stage.color2 = Color::fromYAML(node["color2"]);
        return stage;
    }
};

struct Config {
    std::map<std::string, Stage> stages;

    // Convert Config to/from YAML
    YAML::Node toYAML() const {
        YAML::Node node;
        for (const auto& [key, stage] : stages) {
            node[key] = stage.toYAML();
        }
        return node;
    }

    static Config fromYAML(const YAML::Node& node) {
        Config config;
        for (const auto& item : node) {
            std::string key = item.first.as<std::string>();
            config.stages[key] = Stage::fromYAML(item.second);
        }
        return config;
    }
};

void writeConfigToFile(const Config& config, const std::string& filename) {
    YAML::Node yamlNode = config.toYAML();
    std::ofstream fout(filename);
    fout << yamlNode;
}

Config readConfigFromFile(const std::string& filename) {
    YAML::Node yamlNode = YAML::LoadFile(filename);
    return Config::fromYAML(yamlNode);
}

int main() {
    // Create a sample configuration
    Config config;
    config.stages["stage1"] = {4000, {0, 255, 0}, {0, 0, 0}};
    config.stages["stage2"] = {8000, {255, 0, 0}, {0, 255, 255}};

    // Write configuration to a file
    writeConfigToFile(config, "foo_config.yaml");

    // Read configuration from the file
    Config loadedConfig = readConfigFromFile("foo_config.yaml");

    // Output loaded configuration
    for (const auto& [key, stage] : loadedConfig.stages) {
        std::cout << "Stage: " << key << ", Distance: " << stage.distance 
                  << ", Color1: (" << stage.color1.r << ", " << stage.color1.g 
                  << ", " << stage.color1.b << "), Color2: (" 
                  << stage.color2.r << ", " << stage.color2.g 
                  << ", " << stage.color2.b << ")\n";
    }

    return 0;
}

/// int main() {
///     // Create an inventory with some items and attributes
///     Inventory inventory;
///     inventory.items.push_back({"Health Potion", 10});
///     inventory.items.push_back({"Mana Potion", 5});
///     inventory.attributes["Owner"] = "Hero";
///     inventory.attributes["Location"] = "Castle";
/// 
///     // Serialize the inventory to YAML
///     YAML::Node yamlNode = inventory.toYAML();
///     
///     // Output the serialized YAML
///     std::cout << "Serialized YAML:\n" << yamlNode << std::endl;
/// 
///     // Deserialize back from YAML
///     Inventory newInventory = Inventory::fromYAML(yamlNode);
/// 
///     // Output the deserialized data
///     std::cout << "Deserialized Inventory:\n";
///     
///     for (const auto& item : newInventory.items) {
///         std::cout << "Item: " << item.name << ", Quantity: " << item.quantity << std::endl;
///     }
/// 
///     for (const auto& [key, value] : newInventory.attributes) {
///         std::cout << key << ": " << value << std::endl;
///     }
/// 
///     return 0;
/// }
