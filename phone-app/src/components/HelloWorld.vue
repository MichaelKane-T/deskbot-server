<template>
  <main class="min-h-screen bg-[#0e0f11] text-[#f0f1f5] font-sans antialiased pb-10">
    <div class="pointer-events-none fixed top-0 left-1/2 -translate-x-1/2 w-[500px] h-[300px]"
      style="background:radial-gradient(ellipse at top, rgba(91,141,238,0.1) 0%, transparent 65%); z-index:0" />

    <section class="relative z-10 mx-auto max-w-[420px] px-4 pt-12 space-y-3">
      <div class="flex items-center justify-between pb-1">
        <div class="flex items-center gap-2.5">
          <div class="w-9 h-9 rounded-xl flex items-center justify-center text-lg"
            style="background:rgba(91,141,238,0.12); border:1px solid rgba(91,141,238,0.2)">🤖</div>
          <div>
            <div class="text-[18px] font-semibold tracking-[-0.04em]">desk<b class="text-[#5b8dee]">bot</b></div>
            <div class="text-[10px] text-[#53565f] tracking-wide">planner + focus</div>
          </div>
        </div>

        <div class="flex items-center gap-2">
          <div class="text-sm font-semibold tabular-nums text-[#8b8f9e]">{{ state.time }}</div>
          <div class="flex items-center gap-1.5 px-2.5 py-1 rounded-full text-[10px] font-medium"
            :class="online ? 'text-[#4fd1a5]' : 'text-[#53565f]'"
            :style="online
              ? 'background:rgba(79,209,165,0.08); border:1px solid rgba(79,209,165,0.15)'
              : 'background:rgba(83,86,95,0.1); border:1px solid rgba(83,86,95,0.2)'">
            <span class="w-1.5 h-1.5 rounded-full" :class="online ? 'bg-[#4fd1a5] animate-pulse' : 'bg-[#53565f]'" />
            {{ online ? 'Pico live' : 'offline' }}
          </div>
        </div>
      </div>

      <div class="relative rounded-[20px] overflow-hidden p-5"
        style="background:linear-gradient(135deg,#192040 0%,#141b30 100%); border:1px solid rgba(91,141,238,0.18)">
        <div class="pointer-events-none absolute -bottom-10 -right-8 w-48 h-48 rounded-full"
          style="background:radial-gradient(circle,rgba(91,141,238,0.12),transparent 65%)" />

        <div class="flex items-start justify-between mb-3">
          <span class="text-[10px] font-medium uppercase tracking-[0.14em] text-[#5b8dee] opacity-70">Now active</span>
          <span class="text-[10px] font-semibold px-2 py-0.5 rounded-full"
            :class="state.running
              ? 'text-[#4fd1a5] bg-[rgba(79,209,165,0.1)] border border-[rgba(79,209,165,0.2)]'
              : 'text-[#8b8f9e] bg-[rgba(139,143,158,0.08)] border border-[rgba(139,143,158,0.15)]'">
            {{ state.mode }}
          </span>
        </div>

        <div class="flex items-center gap-3 mb-4">
          <div class="flex-shrink-0 w-14 h-14 rounded-2xl flex items-center justify-center text-xl font-black select-none"
            :class="state.running
              ? 'text-[#4fd1a5] bg-[rgba(79,209,165,0.08)] border border-[rgba(79,209,165,0.2)]'
              : 'text-[#8aacf0] bg-[rgba(91,141,238,0.08)] border border-[rgba(91,141,238,0.15)]'">
            {{ botFace }}
          </div>

          <div class="min-w-0 flex-1">
            <div class="text-[15px] font-medium leading-[1.4] tracking-[-0.015em] truncate">
              {{ state.active_task || 'No task selected' }}
            </div>
            <div class="text-[12px] mt-0.5" :class="state.running ? 'text-[#fb923c]' : 'text-[#8b8f9e]'">
              {{ formatTime(state.seconds_left) }} remaining
            </div>
          </div>
        </div>

        <div class="h-[3px] rounded-full mb-3 overflow-hidden" style="background:rgba(255,255,255,0.06)">
          <div class="h-full rounded-full transition-all duration-1000"
            :style="`width:${pomoPercent}%; background:${state.running ? '#5b8dee' : 'rgba(91,141,238,0.35)'}`" />
        </div>

        <div class="flex items-center justify-between">
          <span class="text-[11px] text-[#53565f]">
            {{ activeIndex + 1 }} of {{ state.tasks.length || 1 }}
          </span>
          <div class="flex items-center gap-1 text-[10px] font-medium text-[#8aacf0] px-2 py-1 rounded-lg"
            style="background:rgba(91,141,238,0.08); border:1px solid rgba(91,141,238,0.15)">
            Synced
          </div>
        </div>
      </div>

      <div class="grid grid-cols-4 gap-2">
        <button @click="post('/toggle')"
          class="col-span-2 flex items-center justify-center gap-1.5 py-3 rounded-2xl text-sm font-semibold transition-all active:scale-95"
          :class="state.running
            ? 'bg-[rgba(251,146,60,0.12)] text-[#fb923c] border border-[rgba(251,146,60,0.25)]'
            : 'bg-[rgba(79,209,165,0.1)] text-[#4fd1a5] border border-[rgba(79,209,165,0.22)]'">
          {{ state.running ? 'Pause' : 'Start Focus' }}
        </button>

        <button @click="post('/next')" class="control-btn">Next →</button>
        <button @click="post('/done')" class="control-btn hover:!text-[#4fd1a5]">Done ✓</button>
      </div>

      <div class="flex rounded-xl p-1 gap-1" style="background:#17181c; border:1px solid #272930">
        <button v-for="t in TABS" :key="t" @click="tab = t"
          class="flex-1 py-2 rounded-lg text-[12px] font-medium transition-all duration-150"
          :class="tab === t ? 'bg-[#1e2026] text-[#f0f1f5] shadow' : 'text-[#53565f] hover:text-[#8b8f9e]'">
          {{ t }}
        </button>
      </div>

      <div v-if="tab === 'Tasks'">
        <div class="flex items-center justify-between px-1 pb-1">
          <span class="text-[11px] font-medium text-[#53565f] uppercase tracking-[0.08em]">Tasks</span>
          <span class="text-[11px] text-[#53565f]">{{ pendingCount }} left</span>
        </div>

        <ul class="space-y-1.5 list-none pb-28">
          <li v-for="(t, i) in state.tasks" :key="i"
            class="relative flex items-center gap-3 px-4 py-3.5 rounded-[14px] transition-all duration-150 active:scale-[0.988]"
            :class="t.done ? 'opacity-30' : activeIndex === i ? 'bg-[#191c26]' : 'bg-[#17181c]'"
            :style="!t.done && activeIndex === i ? 'border:1px solid rgba(91,141,238,0.28)' : 'border:1px solid #272930'">

            <div v-if="activeIndex === i && !t.done"
              class="absolute left-0 w-[3px] rounded-r-sm bg-[#5b8dee]" style="top:20%;bottom:20%" />

            <button class="flex-shrink-0 w-[22px] h-[22px] flex items-center justify-center rounded-[7px]"
              :style="t.done
                ? 'background:#4fd1a5;border:1.5px solid #4fd1a5'
                : activeIndex === i
                  ? 'background:transparent;border:1.5px solid #5b8dee'
                  : 'background:transparent;border:1.5px solid #272930'"
              @click.stop="post('/done')">
              <span v-if="t.done" class="text-[11px] font-black">✓</span>
            </button>

            <div class="flex-1 min-w-0">
              <div class="text-sm leading-[1.4] tracking-[-0.01em] truncate"
                :class="t.done ? 'line-through text-[#53565f]' : 'text-[#f0f1f5]'">
                {{ t.title }}
              </div>
            </div>

            <span v-if="activeIndex === i" class="text-[10px] text-[#8aacf0]">active</span>
          </li>

          <li v-if="!state.tasks.length" class="py-12 text-center list-none">
            <div class="text-3xl mb-2">📋</div>
            <p class="text-sm text-[#53565f]">No tasks yet — add one below</p>
          </li>
        </ul>

        <div class="fixed bottom-0 left-1/2 -translate-x-1/2 w-full max-w-[420px] px-4 pb-8 pt-3 z-20"
          style="background:linear-gradient(to top,#0e0f11 55%,transparent)">
          <div class="flex items-center gap-2 rounded-[14px] pr-1.5 pl-4 transition-all duration-150"
            :style="inputFocused
              ? 'background:#17181c;border:1px solid rgba(91,141,238,0.4);box-shadow:0 0 0 3px rgba(91,141,238,0.07)'
              : 'background:#17181c;border:1px solid #272930'">
            <input v-model="newTitle" placeholder="Add a task…"
              class="flex-1 bg-transparent border-none outline-none text-sm text-[#f0f1f5] py-2.5 placeholder-[#53565f]"
              @keyup.enter="addTask"
              @focus="inputFocused=true"
              @blur="inputFocused=false" />
            <button @click="addTask"
              class="w-9 h-9 flex-shrink-0 flex items-center justify-center rounded-[10px] bg-[#5b8dee] transition-all hover:bg-[#6f9ef0] active:scale-90">
              +
            </button>
          </div>
        </div>
      </div>

      <div v-if="tab === 'Notes'" class="space-y-3">
        <div class="flex items-center justify-between px-1">
          <span class="text-[11px] font-medium text-[#53565f] uppercase tracking-[0.08em]">Notes</span>
          <button @click="saveNotes" class="control-btn px-3 py-1.5">Save</button>
        </div>

        <textarea v-model="notes" placeholder="Ideas, reminders, project notes…"
          class="w-full h-56 resize-none rounded-[16px] px-4 py-4 text-sm text-[#f0f1f5] placeholder-[#53565f] leading-relaxed outline-none"
          style="background:#17181c; border:1px solid #272930" />
      </div>

      <div v-if="tab === 'Ask'" class="space-y-3">
        <div class="rounded-[20px] p-5 space-y-4" style="background:#17181c;border:1px solid rgba(139,92,246,0.18)">
          <div class="flex items-center gap-3">
            <div class="w-10 h-10 rounded-xl flex items-center justify-center text-lg"
              style="background:rgba(139,92,246,0.1);border:1px solid rgba(139,92,246,0.2)">🤖</div>
            <div>
              <div class="text-sm font-semibold">Ask DeskBot</div>
              <div class="text-[11px] text-[#53565f]">LLM planner — coming soon</div>
            </div>
          </div>

          <div class="flex gap-2">
            <input v-model="askText" placeholder="Ask what to work on next…" disabled
              class="flex-1 min-w-0 rounded-xl px-4 py-3 text-sm text-[#53565f] outline-none"
              style="background:#0e0f11;border:1px solid #272930" />
            <button disabled class="px-4 rounded-xl text-sm font-medium text-[#53565f] cursor-not-allowed"
              style="background:#1e2026;border:1px solid #272930">Ask</button>
          </div>

          <p class="text-[11px] text-[#53565f]">
            Will connect to the server-side LLM for smart planning and spoken replies.
          </p>
        </div>
      </div>
    </section>
  </main>
</template>

<script setup>
import { computed, onMounted, ref } from "vue";

const API = "/api/deskbot";
const TABS = ["Tasks", "Notes", "Ask"];
const POMO = 1500;

const tab = ref("Tasks");
const online = ref(false);
const inputFocused = ref(false);
const newTitle = ref("");
const notes = ref("");
const askText = ref("");

const state = ref({
  time: "--:--",
  mode: "IDLE",
  mood: "CALM",
  active_task: "Loading...",
  running: false,
  seconds_left: 1500,
  tasks: [],
  notes: ""
});

const activeIndex = computed(() => {
  const idx = state.value.tasks.findIndex(t => t.title === state.value.active_task);
  return idx >= 0 ? idx : 0;
});

const pendingCount = computed(() =>
  state.value.tasks.filter(t => !t.done).length
);

const pomoPercent = computed(() =>
  Math.max(0, Math.min(100, (state.value.seconds_left / POMO) * 100))
);

const botFace = computed(() => {
  if (state.value.mood === "DONE") return "^_^";
  if (state.value.running) return "◉_◉";
  return "•‿•";
});

function formatTime(s) {
  return `${Math.floor(s / 60)}:${String(s % 60).padStart(2, "0")}`;
}

async function loadState() {
  try {
    const res = await fetch(`${API}/state`);
    const data = await res.json();

    state.value = {
      ...state.value,
      ...data,
      tasks: data.tasks || state.value.tasks,
      notes: data.notes ?? state.value.notes
    };

    notes.value = state.value.notes || "";
    online.value = true;
  } catch {
    online.value = false;
  }
}

async function post(path, body = {}) {
  try {
    await fetch(`${API}${path}`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(body)
    });
    online.value = true;
  } catch {
    online.value = false;
  }

  await loadState();
}

async function addTask() {
  const title = newTitle.value.trim();
  if (!title) return;

  await post("/tasks/add", { title });
  newTitle.value = "";
}

async function saveNotes() {
  await post("/notes", { notes: notes.value });
}

onMounted(() => {
  loadState();
  setInterval(loadState, 5000);
});
</script>

<style>
@import url('https://fonts.googleapis.com/css2?family=Sora:wght@300;400;500;600&display=swap');

body {
  font-family: 'Sora', sans-serif;
  background: #0e0f11;
}

.control-btn {
  background: #17181c;
  border: 1px solid #272930;
  color: #8b8f9e;
  border-radius: 1rem;
  font-size: 12px;
  font-weight: 500;
  transition: all 150ms ease;
}

.control-btn:hover {
  color: #f0f1f5;
  background: #1e2026;
}
</style>