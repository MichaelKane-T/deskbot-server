require("dotenv").config();

const express = require("express");
const cors = require("cors");
const session = require("express-session");
const fs = require("fs");
const path = require("path");

const app = express();
const PORT = 8080;
const DATA_FILE = path.join(__dirname, "data", "deskbot-state.json");

app.use(cors());
app.use(express.json());

app.use(
  session({
    secret: process.env.SESSION_SECRET || "dev_secret_change_me",
    resave: false,
    saveUninitialized: false,
    cookie: {
      httpOnly: true,
      sameSite: "lax",
      secure: false,
      maxAge: 1000 * 60 * 60 * 24 * 14
    }
  })
);

app.use(express.static("phone-app/dist"));

function defaultState() {
  return {
    mode: "IDLE",
    mood: "CALM",
    active_index: 0,
    tasks: [
      { title: "Work on RoverPi", done: false },
      { title: "Fix DeskBot flicker", done: false },
      { title: "Test touch controls", done: false }
    ],
    notes: "",
    pomodoro: {
      running: false,
      seconds_left: 1500
    }
  };
}

function loadState() {
  try {
    fs.mkdirSync(path.dirname(DATA_FILE), { recursive: true });

    if (!fs.existsSync(DATA_FILE)) {
      const initial = defaultState();
      fs.writeFileSync(DATA_FILE, JSON.stringify(initial, null, 2));
      return initial;
    }

    return {
      ...defaultState(),
      ...JSON.parse(fs.readFileSync(DATA_FILE, "utf8"))
    };
  } catch (err) {
    console.error("Failed to load state:", err);
    return defaultState();
  }
}

let state = loadState();

function saveState() {
  fs.writeFileSync(DATA_FILE, JSON.stringify(state, null, 2));
}

function normalizeActiveIndex() {
  if (!state.tasks.length) {
    state.active_index = 0;
    return;
  }

  if (state.active_index < 0) state.active_index = 0;
  if (state.active_index >= state.tasks.length) state.active_index = 0;
}

function activeTask() {
  normalizeActiveIndex();
  return state.tasks[state.active_index]?.title || "No Task";
}

function currentTime() {
  return new Date().toLocaleTimeString("en-US", {
    hour: "numeric",
    minute: "2-digit",
    hour12: true,
    timeZone: "America/Denver"
  });
}

function isLoggedIn(req) {
  return req.session && req.session.loggedIn === true;
}

function requireLogin(req, res, next) {
  if (isLoggedIn(req)) return next();
  return res.status(401).json({ ok: false, error: "Not logged in" });
}

// Auth
app.get("/api/auth/status", (req, res) => {
  res.json({
    loggedIn: isLoggedIn(req),
    user: isLoggedIn(req) ? process.env.DESKBOT_USER : null
  });
});

app.post("/api/auth/login", (req, res) => {
  const { username, password } = req.body || {};

  if (
    username === process.env.DESKBOT_USER &&
    password === process.env.DESKBOT_PASSWORD
  ) {
    req.session.loggedIn = true;
    return res.json({ ok: true });
  }

  return res.status(401).json({ ok: false, error: "Invalid login" });
});

app.post("/api/auth/logout", (req, res) => {
  req.session.destroy(() => {
    res.json({ ok: true });
  });
});

// DeskBot API
app.get("/api/deskbot/state", requireLogin, (req, res) => {
  normalizeActiveIndex();

  res.json({
    time: currentTime(),
    mode: state.mode,
    mood: state.mood,
    active_index: state.active_index,
    active_task: activeTask(),
    running: state.pomodoro.running,
    seconds_left: state.pomodoro.seconds_left,
    tasks: state.tasks,
    notes: state.notes
  });
});

app.post("/api/deskbot/state", requireLogin, (req, res) => {
  state = {
    ...state,
    ...req.body,
    pomodoro: {
      ...state.pomodoro,
      ...(req.body.pomodoro || {})
    }
  };

  normalizeActiveIndex();
  saveState();

  res.json({ ok: true, state });
});

app.post("/api/deskbot/toggle", requireLogin, (req, res) => {
  state.pomodoro.running = !state.pomodoro.running;
  state.mode = state.pomodoro.running ? "FOCUS" : "IDLE";
  state.mood = state.pomodoro.running ? "FOCUS" : "CALM";

  saveState();
  res.json({ ok: true, state });
});

app.post("/api/deskbot/next", requireLogin, (req, res) => {
  if (state.tasks.length > 0) {
    state.active_index = (state.active_index + 1) % state.tasks.length;
  }

  normalizeActiveIndex();
  saveState();

  res.json({ ok: true, state });
});

app.post("/api/deskbot/done", requireLogin, (req, res) => {
  const task = state.tasks[state.active_index];

  if (task) task.done = true;

  state.mood = "DONE";
  saveState();

  setTimeout(() => {
    state.mood = state.pomodoro.running ? "FOCUS" : "CALM";
    saveState();
  }, 3000);

  res.json({ ok: true, state });
});

app.post("/api/deskbot/reset", requireLogin, (req, res) => {
  state.pomodoro.running = false;
  state.pomodoro.seconds_left = 1500;
  state.mode = "IDLE";
  state.mood = "CALM";

  saveState();
  res.json({ ok: true, state });
});

app.post("/api/deskbot/tasks/add", requireLogin, (req, res) => {
  const title = req.body?.title?.trim();

  if (!title) {
    return res.status(400).json({ ok: false, error: "Missing task title" });
  }

  state.tasks.push({ title, done: false });

  normalizeActiveIndex();
  saveState();

  res.json({ ok: true, state });
});

app.post("/api/deskbot/notes", requireLogin, (req, res) => {
  state.notes = req.body?.notes || "";
  saveState();

  res.json({ ok: true, state });
});

app.listen(PORT, "0.0.0.0", () => {
  console.log(`DeskBot server running on http://0.0.0.0:${PORT}`);
  console.log(`State file: ${DATA_FILE}`);
});