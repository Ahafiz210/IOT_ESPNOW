const express = require("express");
const mysql = require("mysql2/promise");

const app = express();

app.use(express.json());

const db_config = {
    host: "127.0.0.1",
    port: 3306,
    user: "espuser",
    password: "esp1234",
    database: "station"
};

// einfacher Zustand im Speicher
const rgb_state = {
    mode: "AUTO",   // AUTO oder MANUAL
    r: 0,
    g: 0,
    b: 0
};

const API_KEY = "mein_geheimer_key";

function check_api_key(req) {
    return req.headers["x-api-key"] === API_KEY;
}

app.get("/", (req, res) => {
    res.send("API läuft");
});

app.get("/insert", async (req, res) => {
    const sensor = req.query.sensor;
    const timestamp = req.query.timestamp;
    const bright = req.query.bright;

    if (sensor === undefined) {
        return res.status(400).json({ error: "sensor fehlt" });
    }

    const status = bright === "1" ? "HELL" : "DUNKEL";

    let conn;

    try {
        conn = await mysql.createConnection(db_config);

        const sql = `
            INSERT INTO messdaten (timestamp, sensor, status)
            VALUES (?, ?, ?)
        `;

        await conn.execute(sql, [
            timestamp,
            parseFloat(sensor),
            status
        ]);

        await conn.end();

        return res.json({
            ok: true,
            sensor: parseFloat(sensor),
            timestamp: timestamp,
            status: status
        });

    } catch (err) {
        if (conn) await conn.end();

        return res.status(500).json({
            error: "datenbankfehler",
            details: err.message
        });
    }
});

app.get("/status", async (req, res) => {
    let conn;

    try {
        conn = await mysql.createConnection(db_config);

        const sql = `
            SELECT id, timestamp, sensor, status
            FROM messdaten
            ORDER BY id DESC
            LIMIT 1
        `;

        const [rows] = await conn.execute(sql);

        await conn.end();

        if (rows.length === 0) {
            return res.status(404).json({ error: "keine daten vorhanden" });
        }

        return res.json(rows[0]);

    } catch (err) {
        if (conn) await conn.end();

        return res.status(500).json({
            error: "datenbankfehler",
            details: err.message
        });
    }
});

app.get("/rgb", (req, res) => {
    res.json(rgb_state);
});

app.post("/rgb/set", (req, res) => {
    if (!check_api_key(req)) {
        return res.status(401).json({ error: "unauthorized" });
    }

    const data = req.body || {};

    let r, g, b;

    try {
        r = parseInt(data.r ?? 0);
        g = parseInt(data.g ?? 0);
        b = parseInt(data.b ?? 0);

        if (isNaN(r) || isNaN(g) || isNaN(b)) {
            throw new Error("ungueltige RGB-Werte");
        }

    } catch (err) {
        return res.status(400).json({ error: "ungueltige RGB-Werte" });
    }

    r = Math.max(0, Math.min(255, r));
    g = Math.max(0, Math.min(255, g));
    b = Math.max(0, Math.min(255, b));

    rgb_state.mode = "MANUAL";
    rgb_state.r = r;
    rgb_state.g = g;
    rgb_state.b = b;

    return res.json({
        ok: true,
        message: "RGB gesetzt",
        state: rgb_state
    });
});

app.post("/rgb/mode", (req, res) => {
    if (!check_api_key(req)) {
        return res.status(401).json({ error: "unauthorized" });
    }

    const data = req.body || {};
    const mode = String(data.mode || "").toUpperCase();

    if (!["AUTO", "MANUAL"].includes(mode)) {
        return res.status(400).json({
            error: "mode muss AUTO oder MANUAL sein"
        });
    }

    rgb_state.mode = mode;

    return res.json({
        ok: true,
        message: `Modus auf ${mode} gesetzt`,
        state: rgb_state
    });
});

app.listen(8000, "0.0.0.0", () => {
    console.log("Server läuft auf http://0.0.0.0:8000");
});
