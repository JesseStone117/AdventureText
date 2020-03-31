#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <time.h>
#include <chrono>
#include <thread>
#include <windows.h>
#include <future>
#include <conio.h>
#include <assert.h>
using namespace std;
using namespace chrono;

// Starting player stats
const int startingHealthCap = 8;
const int startingSustenance = 4;
const int startingStamina = 4;
const int startingInventoryCap = 4;
const int startingDamage = 2;
const int startingSpeed = 4;

const enum itemsEnum { mHealthPot, bread, beegHealthPot, ranch, wildBerries };
const enum enemiesEnum { goblin, ogre , brownie };
const enum destinationsEnum { crossroads, town, wilderness };

//#define startingAuth 1872;


// original authentication value: ((6*2)+(4*2)+(4))*117 = 2808

// (5+7+10+0+0)*117
// (hero.currentHealth + hero.healthCap + hero.stamina + gameInfo.bossDefeated + gameInfo.newPlayer) * 117;

class enemyInfo
{
public:
	string name = "";
	string encounterMessage = "";
	int health = 0;
	int damage = 0;
	int speed = 0;
	int goldMultiplier = 0;
	int ID = 0;
};

class ItemInfo
{
public:
	string name = "";
	string usageMessage = "";
	int value = 0;
	int count = 0;
	int cost = 0;
	int ID = -1;
	// int weight;
};

class Inventory
{
public:
	int count = 0;
	int ID = -1;
};

class Player
{
public:
	int healthCap = startingHealthCap;
	int currentHealth = healthCap;
	int sustenance = startingSustenance;
	int currentSustenance = startingSustenance;
	int stamina = startingStamina;
	int currentStamina = stamina;
	int inventoryCap = startingInventoryCap;
	int baseDamage = startingDamage;
	int currentInventory = 0;
	int speed = startingSpeed;
	int gold = 0;
	int currentDestination = crossroads;
	Inventory slots[12];

};

class GameInfo
{
public:
	bool newPlayer = 1;
	bool bossDefeated = 0;
};

string operator*(string const&, size_t);


void quickTime(std::future<void> futureObj, int*, int, int);

void saveGame(Player&, GameInfo&);
void startUp(Player&, GameInfo&);
void displayUI(Player);
bool displayInventory(Player&, GameInfo&);
bool displayShopkeep(Player&, GameInfo&);
void purchaseItem(Player&, GameInfo&, int);

string health(Player);
string sustenance(Player);
string stamina(Player);

string printEnemyHealth(int, int);

bool consumeSustenance(Player&, GameInfo&, int);
bool consumeStamina(Player&, GameInfo&, int);
void consumeFood(Player&, GameInfo&, int);
void acquireItem(Player&, GameInfo&, int, int);

void brawlerEncounter(Player&, GameInfo&, int);
void block(int&, int&, int, int);
void attack(int&, int&, int);

void damagePlayer(Player&, GameInfo&, int);
void drinkPotion(Player&, GameInfo&, int);

void travelSequence(Player&, GameInfo&);
void destination(Player&, GameInfo&, int);
void enterTown(Player&, GameInfo&);
void enterWilderness(Player&, GameInfo&);
void enterCrossroads(Player&, GameInfo&);
void lowerCase(string&);

bool deathProcess(Player&, GameInfo&);


const enemyInfo enemies[20] // name, encounterMessage, health, damage, speed, goldMultiplier, ID
{
	// Goblin
	"goblin", " You're challenged by a ravenous goblin!", 3, 2, 5, 1, goblin,
	"ogre", " You're challenged by a lumbering ogre!", 6, 4, 1, 3, ogre,
	"brownie", "You're challenged by a weakly brownie... so scary", 2, 2, 2, 1, brownie
};

const ItemInfo items[20] // name, usageMessage, value, count, cost, ID
{
	// Medium health potion
	"Medium Health Potion", "[D]rink health potion", 3, 0, 50, mHealthPot,

	// Loaf of bread
	"Loaf of Bread", "[E]at some food", 2, 0, 25, bread,

	// Beeg health potion
	"Beeg Health Potion", "[S]lurp beeg potion", 6, 0, 90, beegHealthPot,

	// Ranch slab...
	"Slab of Ranch", "[G]uzzle some ranch", 1, 0, 20, ranch,

	// Wild berries
	"Wild Berries", "[M]onch some berries", 2, 0, 40, wildBerries
};

int main()
{
	Player hero;
	GameInfo gameInfo;
	srand(_getpid());

	startUp(hero, gameInfo);

	cout << "\n\n\n Welcome back traveler. Press enter to resume your adventure...";

	loop:
		cin.get();

		while (hero.currentHealth > 0)
		{
			displayUI(hero);
			destination(hero, gameInfo, hero.currentDestination);
		}

		system("CLS");

		cout << "\n\n\n You died...";
		cin.get();

		while (!deathProcess(hero, gameInfo));

	goto loop;
}

void startUp(Player& hero, GameInfo& gameInfo)
{
	ifstream infile;

	// Load game info
	infile.open("gameInfo.txt");

	infile.read((char*)&gameInfo, sizeof(gameInfo));

	infile.close();
	
	// Set authCheck equal to the current authentication value
	
	if (gameInfo.newPlayer == true)
	{
		// Load new player info
		for (int i = 0; i < startingInventoryCap; i++)
		{
			hero.slots[i].ID = -1; hero.slots[i].count = 0;
		}

		// Load new game info
		gameInfo.newPlayer = 0;

		acquireItem(hero, gameInfo, 2, bread);

		saveGame(hero, gameInfo);
	}
	else
	{
		// Load player info
		infile.open("playerInfo.txt");
		
		infile.read((char*)&hero, sizeof(hero));

		infile.close();
	}


	// Calculate a current authentication value based on current player data
	saveGame(hero, gameInfo);

	
}


void saveGame(Player& hero, GameInfo& gameInfo)
{
	ofstream outfile;

	// Save player info
	outfile.open("playerInfo.txt");

	outfile.write((char*)&hero, sizeof(hero));

	outfile.close();

	// Save game info
	outfile.open("gameInfo.txt");

	outfile.write((char*)&gameInfo, sizeof(gameInfo));

	outfile.close();
}


void damagePlayer(Player& hero, GameInfo& gameInfo, int damage)
{
	if ((hero.currentHealth - damage) < 0)
	{
		damage = hero.currentHealth;
		hero.currentHealth -= damage;
		displayUI(hero);
		saveGame(hero, gameInfo);
	}
	else
	{
		hero.currentHealth -= damage;

		displayUI(hero);

		cout << " You took " << damage << " damage" << endl;

		saveGame(hero, gameInfo);
	}
}

void displayUI(Player hero)
{
	string printHealth = health(hero);
	string printSustenance = sustenance(hero);
	string printStamina = stamina(hero);
	
	int goldLength;

	goldLength = to_string(hero.gold).length();


	system("CLS");

	cout << setw(119 - printHealth.length()) << "Health: " << printHealth;

	cout << "\n";

	cout << setw(119 - printSustenance.length()) << "Sustenance: " << printSustenance;

	cout << "\n";

	cout << setw(119 - printStamina.length()) << "Stamina: " << printStamina;

	cout << "\n";

	cout << setw(119 - goldLength) << "Gold: " << hero.gold;

	cout << "\n";

}

bool displayInventory(Player& hero, GameInfo& gameInfo)
{
	string selection = "0";
	char select = '0';

	int e = -1;
	int g = -1;
	int s = -1;
	int d = -1;
	int m = -1;

	int countSpacing = 0;

	string usageMessagePrint;
	string namePrint;

	displayUI(hero);

	cout << " {Inventory}\n\n";

	cout << setw(16) << "Equipment:";

	cout << "\n";
	cout << setw(41) << "-----------------------------------";
	cout << "\n";

	for (int i = 0; i < hero.inventoryCap; i++)
	{
		if (hero.slots[i].count > 0)
		{
			countSpacing = to_string(hero.slots[i].count).length();
			
			usageMessagePrint = items[hero.slots[i].ID].usageMessage;
			usageMessagePrint.append(" | ");
			cout << setw(8) << " | " << items[hero.slots[i].ID].name << " <" << items[hero.slots[i].ID].value << ">";
			cout << " (x" << hero.slots[i].count << ")" << setw(25 - countSpacing - items[hero.slots[i].ID].name.length()) << "|" << "\n";
			cout << setw(41) << "-----------------------------------";
			cout << "\n";

			switch (hero.slots[i].ID)
			{
			case mHealthPot:
				d = i;
				break;
			case bread:
				e = i;
				break;
			case beegHealthPot:
				s = i;
				break;
			case ranch:
				g = i;
				break;
			case wildBerries:
				m = i;
				break;
			default:
				break;
			}
		}
		else
		{
			cout << setw(8) << " | " << "                                |" << "\n";
			usageMessagePrint = "";
			cout << setw(41) << "-----------------------------------";
			cout << "\n";
		}
	}

	cout << (string("\n") * 9);

	cout << " Options: ";

	for (int i = 0; i < hero.inventoryCap; i++)
	{
		if (hero.slots[i].count > 0)
		{
			cout << items[hero.slots[i].ID].usageMessage << " | ";
		}

	}
	cout << "[C]lose inventory";
	cout << "\n I want to ";

	getline(cin, selection);

	lowerCase(selection);

	select = selection[0];


	switch (select)
	{
	case 'c':
		displayUI(hero);
		return false;
		break;

	case 'e':
		if (e == -1) return true;

		consumeFood(hero, gameInfo, bread);
		break;

	case 'g':
		if (g == -1) return true;

		consumeFood(hero, gameInfo, ranch);
		break;

	case 'm':
		if (m == -1) return true;

		consumeFood(hero, gameInfo, wildBerries);
		break;

	case 's':
		if (s == -1) return true;

		drinkPotion(hero, gameInfo, beegHealthPot);
		break;

	case 'd':
		if (d == -1) return true;

		drinkPotion(hero, gameInfo, mHealthPot);
		break;

	default: break;
	}

	return true;
}

bool displayShopkeep(Player& hero, GameInfo& gameInfo)
{

	string selection = "0";

	char select = '0';

	const int storeSelection = 3;

	// current date/time based on current system
	time_t now = time(0);

	tm* ltm = localtime(&now);


	int shopItems[storeSelection] = { 0 };

	//cout << "Time: " << ltm->tm_hour;


	if (ltm->tm_hour <= 2)
	{
		shopItems[0] = beegHealthPot;
		shopItems[1] = ranch;
		shopItems[2] = mHealthPot;
	}
	else if (ltm->tm_hour < 5)
	{
		shopItems[0] = wildBerries;
		shopItems[1] = bread;
		shopItems[2] = mHealthPot;
	}
	else if (ltm->tm_hour < 10)
	{
		shopItems[0] = bread;
		shopItems[1] = mHealthPot;
		shopItems[2] = wildBerries;
	}
	else
	{
		shopItems[0] = mHealthPot;
		shopItems[1] = bread;
		shopItems[2] = beegHealthPot;
	}


	displayUI(hero);

	cout << " {Shopkeep - Grimlith}\n\n";

	cout << setw(12) << "Wares:";


	cout << "\n";
	cout << setw(58) << "----------------------------------------------------";
	cout << "\n";


	for (int i = 0; i < storeSelection; i++)
	{
		cout << setw(8) << " | " << "Item #" << i + 1 << ": (" << items[shopItems[i]].cost << " gold) - " << items[shopItems[i]].name;
		cout << " <" << items[shopItems[i]].value << "> " << setw(12) << setw(24 - items[shopItems[i]].name.length()) << "|" << "\n";
		cout << setw(58) << "----------------------------------------------------";
		cout << "\n";
	}


	cout << (string("\n") * 11);

	cout << " Options: ";

	for (int i = 0; i < storeSelection; i++)
	{
		cout << "Purchase item #[" << i + 1 << "] | ";
	}

	cout << "[E]nd interaction";
	cout << "\n I want to ";


	getline(cin, selection);

	lowerCase(selection);

	select = selection[0];


	switch (select)
	{
	case 'e':
		displayUI(hero);
		return false;
		break;
	case '1':
		purchaseItem(hero, gameInfo, shopItems[0]);
		break;
	case '2':
		purchaseItem(hero, gameInfo, shopItems[1]);
		break;
	case '3':
		purchaseItem(hero, gameInfo, shopItems[2]);
		break;
	default: break;
	}

	return true;
}

void purchaseItem(Player& hero, GameInfo& gameInfo, int item)
{
	if (hero.gold >= items[item].cost && hero.currentInventory < hero.inventoryCap)
	{
		hero.gold -= items[item].cost;
		acquireItem(hero, gameInfo, 1, item);
		cout << " Acquired 1 " << items[item].name << " [Enter]";
		cin.get();
	}
	else if (hero.gold >= items[item].cost && hero.currentInventory == hero.inventoryCap)
	{
		for (int i = 0; i < hero.inventoryCap; i++)
		{
			if (hero.slots[i].ID == item)
			{
				hero.gold -= items[item].cost;
				acquireItem(hero, gameInfo, 1, item);
				cout << " Acquired 1 " << items[item].name << " [Enter]";
				cin.get();
				return;
			}
		}
		cout << " Inventory is full... [Enter]";
		cin.get();
	}
	else
	{
		cout << " Insufficient funds... [Enter]";
		cin.get();
	}
}

string health(Player hero)
{
	string health = "";


	for (int i = 0; i < hero.healthCap; i++)
	{
		if (hero.currentHealth < hero.healthCap && hero.currentHealth <= i)
		{
			health.append("[X]");
		}
		else
		{
			health.append("[O]");
		}

	}

	return health;
}


string sustenance(Player hero)
{
	string sustenance = "";


	for (int i = 0; i < hero.sustenance; i++)
	{
		if (hero.currentSustenance < hero.sustenance && hero.currentSustenance <= i)
		{
			sustenance.append("[X]");
		}
		else
		{
			sustenance.append("[O]");
		}

	}

	return sustenance;
}


string stamina(Player hero)
{
	string stamina = "";


	for (int i = 0; i < hero.stamina; i++)
	{
		if (hero.currentStamina < hero.stamina && hero.currentStamina <= i)
		{
			stamina.append("[X]");
		}
		else
		{
			stamina.append("[O]");
		}

	}

	return stamina;
}


string printEnemyHealth(int currentHealth, int ID)
{
	string health = "";

	string name = enemies[ID].name;

	name[0] = toupper(name[0]);

	health.append("    ");
	health.append(name);
	health.append(" health: ");

	for (int i = 0; i < enemies[ID].health; i++)
	{
		if (currentHealth < enemies[ID].health && currentHealth <= i)
		{
			health.append("[X]");
		}
		else
		{
			health.append("[O]");
		}

	}

	health.append("\n\n");

	return health;
}


bool consumeSustenance(Player& hero, GameInfo& gameInfo, int sustenanceUsed)
{
	if (hero.currentSustenance >= sustenanceUsed && hero.currentSustenance > 0)
	{
		hero.currentSustenance -= sustenanceUsed;
		displayUI(hero);
		saveGame(hero, gameInfo);
		return true;
	}
	else
	{
		return false;
	}
}


bool consumeStamina(Player& hero, GameInfo& gameInfo, int staminaUsed)
{
	if (hero.currentStamina >= staminaUsed && hero.currentStamina > 0)
	{
		hero.currentStamina -= staminaUsed;
		displayUI(hero);
		saveGame(hero, gameInfo);
		return true;
	}
	else
	{
		if (hero.currentSustenance > 0)
		{
			hero.currentSustenance--;
			hero.currentStamina = hero.stamina;
			return true;
		}
		else
		{
			return false;
		}
	}
}


void drinkPotion(Player& hero, GameInfo& gameInfo, int itemID)
{
	int hpRecovered = items[itemID].value;

	for (int i = 0; i < hero.inventoryCap; i++)
	{
		if (hero.slots[i].ID == itemID && hero.slots[i].count > 0)
		{
			hero.slots[i].count--;
			if (hero.slots[i].count == 0)
			{
				hero.slots[i].ID = -1;
				hero.currentInventory--;
			}
		}
	}

	if ((hero.currentHealth + hpRecovered) > hero.healthCap)
	{
		hpRecovered = hero.healthCap - hero.currentHealth;
	}

	hero.currentHealth += hpRecovered;

	cout << " You recovered " << hpRecovered << " health [Enter]";

	cin.get();
	saveGame(hero, gameInfo);
}

void consumeFood(Player& hero, GameInfo& gameInfo, int itemID)
{
	int staminaRecovered = items[itemID].value;

	for (int i = 0; i < hero.inventoryCap; i++)
	{
		if (hero.slots[i].ID == itemID && hero.slots[i].count > 0)
		{
			hero.slots[i].count--;

			if (hero.slots[i].count == 0)
			{
				hero.slots[i].ID = -1;
				hero.currentInventory--;
			}
		}
	}

	if ((hero.currentStamina + staminaRecovered) > hero.stamina)
	{
		staminaRecovered = hero.stamina - hero.currentStamina;
	}

	hero.currentStamina += staminaRecovered;

	//displayUI(hero);

	cout << " You recovered " << staminaRecovered << " stamina [Enter]";

	cin.get();
	saveGame(hero, gameInfo);
}

void travelSequence(Player& hero, GameInfo& gameinfo)
{
	system("CLS");

	string travelHistory = "";

	int enemyType = goblin;

	cout << (string("\n") * 13);

	cout << setw(60) << "Traveling\n";

	cout << setw(53);

	for (int i = 0; i < 5; i++)
	{
		if (rand() % 10 == 5)
		{
			cout << "x";
			travelHistory.append("x");
			this_thread::sleep_for(chrono::milliseconds(150));

			if (rand() % 2 == 1)
			{
				enemyType = brownie;
			}

			brawlerEncounter(hero, gameinfo, enemyType);

			if (hero.currentHealth == 0) return;

			cout << (string("\n") * 10);

			cout << setw(60) << "Traveling\n";

			cout << setw(53 + i);

			cout << travelHistory;

		}
		else
		{
			cout << "o";
			travelHistory.append("o");
			this_thread::sleep_for(chrono::milliseconds(150));

		}
	}

	//if (rand() % 2 == 1)
	//{
	//	goblinEncounter(hero, gameinfo);
	//}

}

void destination(Player& hero, GameInfo& gameInfo, int currentDestination)
{

	switch (currentDestination)
	{
	case crossroads:
		enterCrossroads(hero, gameInfo);
		break;
	case town:
		enterTown(hero, gameInfo);
		break;
	case wilderness:
		enterWilderness(hero, gameInfo);
	default:
		break;
	}
}

void enterTown(Player& hero, GameInfo& gameInfo)
{
	string selection = "0";
	char select = '0';

	cout << " {Town}";
	cout << "\n\n What would you like to do, traveler?";

	cout << (string("\n") * 19);



	cout << " Options: [R]ight to the crossroads | [A]pproach the shopkeep | [O]pen inventory | [E]xit the game";
	cout << "\n I want to ";

	getline(cin, selection);

	lowerCase(selection);

	select = selection[0];


	switch (select)
	{
	case 'e':
		exit(0);
		break;

	case 'r':
		travelSequence(hero, gameInfo);
		hero.currentDestination = crossroads;
		saveGame(hero, gameInfo);
		break;

	case 'o':
		while (displayInventory(hero, gameInfo) == true);
		break;

	case 'a':
		while (displayShopkeep(hero, gameInfo) == true);
		break;

	default:
		displayUI(hero);
	}
}

void enterWilderness(Player& hero, GameInfo& gameInfo)
{
	string selection = "0";
	char select = '0';

	cout << " {The Wilderness}";
	cout << "\n\n What would you like to do, traveler?";

	cout << (string("\n") * 19);

	cout << " Options: [L]eft to the crossroads | [F]ight a guy | [O]pen inventory | [E]xit the game";
	cout << "\n I want to ";

	getline(cin, selection);

	lowerCase(selection);

	select = selection[0];


	switch (select)
	{
	case 'e':
		exit(0);
		break;
	case 'f':
		brawlerEncounter(hero, gameInfo, ogre);
		break;
	case 'l':
		travelSequence(hero, gameInfo);
		hero.currentDestination = crossroads;
		saveGame(hero, gameInfo);
		break;
	case 'o':
		while (displayInventory(hero, gameInfo) == true);
		break;
	default:
		displayUI(hero);
	}
}

void enterCrossroads(Player& hero, GameInfo& gameInfo)
{
	string selection = "0";
	char select = '0';

	cout << " {Crossroads}";
	cout << "\n\n What would you like to do, traveler?";

	cout << (string("\n") * 19);

	cout << " Options: [L]eft to town | [R]ight to the wilderness | [O]pen inventory | [E]xit the game";
	cout << "\n I want to ";

	getline(cin, selection);

	lowerCase(selection);

	select = selection[0];


	switch (select)
	{
	case 'e':
		exit(0);
		break;
	case 'o':
		while (displayInventory(hero, gameInfo) == true);
		break;
	case 'l':
		travelSequence(hero, gameInfo);
		hero.currentDestination = town;
		saveGame(hero, gameInfo);
		break;
	case 'r':
		travelSequence(hero, gameInfo);
		hero.currentDestination = wilderness;
		saveGame(hero, gameInfo);
		break;
	default:
		displayUI(hero);
	}
}

void lowerCase(string& strToConvert)
{
	for (unsigned int i = 0; i < strToConvert.length(); i++)
	{
		strToConvert[i] = tolower(strToConvert[i]);
	}
}

bool deathProcess(Player& hero, GameInfo& gameInfo)
{
	system("CLS");

	string selection = "0";
	cout << "\n\n\n You must sacrifice one 'Fragment of the Divine' to recover\n\n";
	cout << " Options: [S]acrifice 'Fragment of the Divine' | [E]xit the game";
	cout << "\n I want to ";

	getline(cin, selection);

	lowerCase(selection);

	if (selection == "s")
	{
		hero.currentHealth = hero.healthCap;
		hero.currentStamina = hero.stamina;
		hero.gold = 0;

		for (int i = 0; i < hero.inventoryCap; i++)
		{
			hero.slots[i].ID = -1;
			hero.slots[i].count = 0;
		}

		hero.currentDestination = crossroads;
		hero.currentInventory = 0;
		hero.currentSustenance = hero.sustenance;

		saveGame(hero, gameInfo);

		displayUI(hero);
		cout << " Welcome back to life! Isn't it great to not be dead anymore? [Enter]";

		return true;
	}
	else if (selection == "e")
	{
		exit(0);
	}
	else
	{
		return false;
	}
}

void brawlerEncounter(Player& hero, GameInfo& gameInfo, int enemyType)
{
	enemyInfo enemy = enemies[enemyType];

	//string enemyName = enemies[enemyType].name;
	//string encounterMessage = enemies[enemyType].encounterMessage;
	//const int baseDamage = enemies[enemyType].damage;
	
	//int brawlerHealth = enemies[enemyType].health;
	//int brawlerDamage = enemies[enemyType].damage;
	//int brawlerLoot = enemies[enemyType].goldMultiplier;
	//int brawlerSpeed = enemies[enemyType].speed;

	int finalEnemyDamage = enemy.damage;

	int outcome;
	int stamina = 1;
	bool encounterStart = true;

	displayUI(hero);

	//  _____
	// |     |
	// | \-/ |
	// |  |  |
	//  \___/
	

	//  _||_
	//  |  |
	//  |  |
	//  |  |
	//   \/


	while (enemy.health > 0 && hero.currentHealth > 0)
	{
		int basePlayerDamage = hero.baseDamage;

		if (encounterStart == true)
		{
			encounterStart = false;
			cout << enemy.encounterMessage;
			cout << "\n";

			cout << printEnemyHealth(enemy.health, enemyType);

			cout << " Options: [Enter] to stand and fight\n";
		}
		else
		{
			cout << " The " << enemy.name << " won't seem to give up. Better finish him off before he finishes you...\n";

			cout << printEnemyHealth(enemy.health, enemyType);

			cout << " Options: [Enter] to stand and fight\n";
			finalEnemyDamage = enemy.damage;
		}
		
		cin.get();

		if (hero.speed > enemy.speed)
		{
			attack(stamina, basePlayerDamage, hero.speed);

			consumeStamina(hero, gameInfo, 1);

			enemy.health -= basePlayerDamage;

			if (enemy.health > 0)
			{
				block(stamina, finalEnemyDamage, enemy.damage, enemy.speed);

				consumeStamina(hero, gameInfo, 1);

				damagePlayer(hero, gameInfo, finalEnemyDamage);
			}
		}
		else
		{
			block(stamina, finalEnemyDamage, enemy.damage, enemy.speed);

			consumeStamina(hero, gameInfo, 1);

			attack(stamina, basePlayerDamage, hero.speed);
			
			consumeStamina(hero, gameInfo, 1);

			damagePlayer(hero, gameInfo, finalEnemyDamage);

			enemy.health -= basePlayerDamage;
		}


		if (enemy.health < 1)
		{
			cout << "\n You've slain the " << enemy.name << "! [Enter]";
			hero.gold += (48)*enemy.goldMultiplier;
		}

		cin.get();

		saveGame(hero, gameInfo);

		displayUI(hero);
	}
}

void acquireItem(Player& hero, GameInfo& gameInfo, int count, int ID)
{
	int i = 0;

	bool slotFound = false;

	while (slotFound == false && i < hero.inventoryCap)
	{
		if (hero.slots[i].ID == ID)
		{
			hero.slots[i].count += count;
			hero.slots[i].ID = ID;
			slotFound = true;
		}
		i++;
	}

	i = 0;

	while (slotFound == false && i < hero.inventoryCap)
	{
		if (hero.slots[i].count == 0)
		{
			// Increment current inventory amount only if the slot was empty
			hero.currentInventory++;

			hero.slots[i].count += count;
			hero.slots[i].ID = ID;
			slotFound = true;
		}
		i++;
	}

	saveGame(hero, gameInfo);
}


void block(int& stamina, int& enemyDamage, int baseDamage, int speed)
{
	const int block = 0;

	int quickTimeResult = 0;

	int* quickTimePointer = &quickTimeResult;

	// Create a std::promise object
	promise<void> exitSignal;

	//Fetch std::future object associated with promise
	future<void> futureObj = exitSignal.get_future();

	// Starting Thread & move the future object in lambda function by reference
	thread th(&quickTime, move(futureObj), quickTimePointer, speed, block);


	while (!_getch());
	{
		//cout << "[x]";
		exitSignal.set_value();
	}

	cout << "\n";

	if (quickTimeResult > 50)
	{
		stamina = 0;
	}
	else if (quickTimeResult > 41 && quickTimeResult < 44) // == 43
	{
		cout << " Excellent\n";
		enemyDamage = 0;
	}
	else if (quickTimeResult > 35 && quickTimeResult < 47)
	{
		cout << " Good\n";
		enemyDamage = baseDamage / 2;
	}
	else
	{
		cout << " BAD\n";
		enemyDamage = baseDamage;
	}

	if (quickTimeResult <= 49)
	{
		cin.get();
	}

	//Wait for thread to join
	th.join();
}


void attack(int& stamina, int& basePlayerDamage, int speed)
{
	const int attack = 1;

	int quickTimeResult;

	int* quickTimePointer = &quickTimeResult;

	// Create a std::promise object
	promise<void> exitSignal;

	//Fetch std::future object associated with promise
	future<void> futureObj = exitSignal.get_future();

	// Starting Thread & move the future object in lambda function by reference
	thread th(&quickTime, move(futureObj), quickTimePointer, speed, attack);

	while (!_getch());
	{
		//cout << "[x]";
		exitSignal.set_value();
	}

	
	cout << "\n";

	if (quickTimeResult > 50)
	{
		stamina = 0;
		basePlayerDamage = 0;
	}
	else if (quickTimeResult > 41 && quickTimeResult < 44)
	{
		cout << " Excellent\n";
		basePlayerDamage *= 1.5;
	}
	else if (quickTimeResult > 35 && quickTimeResult < 47)
	{
		cout << " Good\n";
	}
	else
	{
		cout << " BAD\n";
		basePlayerDamage = 0;
	}

	if (quickTimeResult <= 49)
	{
		cin.get();
	}

	th.join();
}


void quickTime(std::future<void> futureObj, int* quickTimePointer, int speed, int action)
{
	int i = 0;
	int finalSpeed = 15 / speed;
	string progressBar = " \\___ ___ ___ ___ ___ ___ ___ ___ ___/     \\/  \\____/";

	if (action == 1)
	{
//  _____
// |     |
// | \-/ |
// |  |  |
//  \___/


//  _|_
//  | |
//  |||
//  |||
//  \|/
		cout << "    _|_        Get ready to attack!\n";
		cout << "    |||\n";
		cout << "    |||\n";
		cout << "    |||\n";
		cout << "    \\|/\n";
	}
	else if (action == 0)
	{
		cout << "     _____       Get ready to block the enemy's attack!\n";
		cout << "    |     |\n";
		cout << "    | \\-/ |\n";
		cout << "    |  |  |\n";
		cout << "     \\___/\n";
	}


	cout << setw(progressBar.length()+3) << "||\n";
	cout << setw(progressBar.length()+10) << progressBar << "\n";


	cout << setw(13);
	while (futureObj.wait_for(milliseconds()) == future_status::timeout && i < 50)
	{
		i++;
		*quickTimePointer = i;
		cout << "|";

		this_thread::sleep_for(milliseconds(15 + finalSpeed));
	}

	/*if (i > 15)
	{
		*quickTimePointer = 20;
	}*/
	
	
	if (action == 0 && *quickTimePointer > 49)
	{
		cout << "\n You failed to block the attack [Enter]\n";
	}
	else if (action == 1 && *quickTimePointer > 49)
	{
		cout << "\n You failed to attack the enemy [Enter]\n";
	}
}


std::string operator*(std::string const& s, std::size_t n)
{
	std::string r;
	r.reserve(n * s.size());
	for (std::size_t i = 0; i < n; ++i)
		r += s;
	return r;
}
