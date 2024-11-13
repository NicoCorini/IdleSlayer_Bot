# IdleSlayerBot

**IdleSlayerBot** is a C++ bot designed to automate farming and progression tasks in the game *Idle Slayer*. By using this bot, players can maximize resource gathering and streamline repetitive actions, making gameplay more efficient and effective.

## Key Features

IdleSlayerBot automates several essential tasks to help players increase their **Coins per Second (CpS)**, gather **Slayer Points (SP)**, and progress faster in *Idle Slayer*:

- **Continuous Farming**: The bot constantly jumps and shoots, ensuring continuous farming of coins, points, and other resources.
- **Automated Chest Hunts**: Starts and partially completes chest hunts, solving the chest minigame (note: does not yet support perfect chest hunts, 2x rewards, or fast animations).
- **Bonus Stage Handling**: Initiates all four types of bonus stages, setting them up for collection and maximization.
- **Equipment Purchasing**: Buys and maxes out the best available equipment, adding slots for optimal CpS gains.
- **Upgrade Purchasing**: Purchases available upgrades, focusing on maximizing CpS without investing in vertical magnets.
- **Quest Claiming**: Automatically claims completed quests for rewards and progression.
- **Automatic Ascension**: Ascends every 30 minutes, refreshing CpS and Slayer Point (SP) gains and enabling the purchase of additional upgrades.
- **Minion Collection**: (Pending feature) Will automate minion collection once "claim all" is unlocked in-game.

The bot is set to perform an **ascend every 30 minutes** to boost CpS and SP, while checking for upgrades and equipment every 10 minutes, or whenever the shop indicator is flashing yellow.

## Recommended Game Settings

For optimal performance with IdleSlayerBot, please configure the game with the following settings:

- **Windowed Mode** at a resolution of **1280x720**
- **Hide Locked Quest Rewards** to prevent interface clutter
- **Disable Custom Cursor** for accurate input simulation
- **Enable Rounded Bulk Buy** for efficient purchasing of upgrades

> **Note**: Since the bot controls the mouse, it is recommended to use it within a virtual machine environment to avoid interruptions on your main desktop.

## Controls

- **Press `Q`**: Quit the bot entirely.
- **Press `P`**: Pause the bot (only effective when it is in the jumping and shooting routine, not during other automated tasks).

## How It Works

The bot leverages key aspects of *Idle Slayer* mechanics, such as Coins per Second (CpS) and Slayer Points (SP), to enhance the player’s progress efficiently:

- **Coins Per Second (CpS)**: A critical metric in *Idle Slayer*, as it drives the rate at which players earn coins, which are essential for purchasing upgrades and equipment. By increasing CpS through frequent ascensions and equipment upgrades, the bot helps ensure continuous growth in earning potential.
- **Frequent Ascension**: Ascending often allows the bot to increase CpS and Slayer Points faster, optimizing the benefits of soul farming, SP rewards, and equipment upgrades. Ascension is scheduled every 30 minutes to maintain steady progression.
- **Equipment and Upgrades**: By purchasing and maxing out the best equipment, the bot increases CpS substantially. It also manages upgrades, skipping vertical magnets and focusing on CpS-boosting items.

## Installation and Usage

1. **Download the executable** from the assets below or **build from source** if needed.
2. **Run `IdleSlayerBot.exe`** and keep *Idle Slayer* open with the recommended settings. The bot will begin automating tasks based on its programmed routine.

## Disclaimer

This bot takes control of the mouse to perform actions, so avoid using your main desktop while it's running or consider using a virtual machine. Additionally, please ensure you are in compliance with game terms and community guidelines when using automation tools.
