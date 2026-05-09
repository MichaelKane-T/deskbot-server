const state = {
  mode: "IDLE",
  mood: "CALM",

  active_index: 0,

  tasks: [
    { title: "Work on RoverPi", done: false },
    { title: "ECEN Homework", done: false },
    { title: "Clean Desk", done: false }
  ],

  pomodoro: {
    running: false,
    seconds_left: 1500
  }
};
