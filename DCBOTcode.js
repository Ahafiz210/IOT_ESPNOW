const { Client, GatewayIntentBits, SlashCommandBuilder, REST, Routes } = require("discord.js");

const TOKEN = "MTQ4MjM1NDgwNjI1MjI0NTAxMg.G_8m2z.JmT_X9ZiFEUkDy0fmiNG6fOkQnsm4BCF8mXlFg";
const CLIENT_ID = "1482354806252245012";
const GUILD_ID = "1482355076554035200";
const STATUS_URL = "http://127.0.0.1:8000/status";

const client = new Client({
    intents: [GatewayIntentBits.Guilds],
});

const commands = [
    new SlashCommandBuilder()
        .setName("status")
        .setDescription("Zeigt den aktuellen Sensorstatus an")
        .toJSON(),
];

async function registerCommands() {
    try {
        const rest = new REST({ version: "10" }).setToken(TOKEN);

        await rest.put(
            Routes.applicationGuildCommands(CLIENT_ID, GUILD_ID),
            { body: commands }
        );

        console.log("Slash-Command erfolgreich registriert.");
    } catch (error) {
        console.error("Fehler beim Registrieren der Commands:", error);
    }
}

client.once("ready", () => {
    console.log(`Bot ist online als ${client.user.tag}`);
});

client.on("interactionCreate", async (interaction) => {
    if (!interaction.isChatInputCommand()) return;

    if (interaction.commandName === "status") {
        try {
            const r = await fetch(STATUS_URL, { method: "GET", timeout: 5000 });

            if (!r.ok) {
                await interaction.reply({
                    content: `Fehler beim Abrufen des Status: HTTP ${r.status}`,
                    ephemeral: true,
                });
                return;
            }

            const data = await r.json();

            const msg =
                `**Aktueller Status**\n` +
                `Status: **${data.status}**\n` +
                `Lichtwert: **${data.sensor}**\n` +
                `Zeit: **${data.timestamp}**`;

            await interaction.reply(msg);
        } catch (e) {
            await interaction.reply({
                content: `Fehler: ${e}`,
                ephemeral: true,
            });
        }
    }
});

(async () => {
    await registerCommands();
    client.login(TOKEN);
})();
