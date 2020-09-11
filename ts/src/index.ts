const addInputListener = (
  inputEl: HTMLTextAreaElement,
  outputEl: HTMLDivElement
) => {
  const keyDownListener = (e: KeyboardEvent) => {
    if (e.ctrlKey && e.code === "Enter") {
      e.preventDefault();
      if (inputEl.value.trim().length === 0) {
        return;
      }
      outputEl.innerHTML += `${inputEl.value.trim()}<br />`;
      outputEl.scrollTo({ top: outputEl.scrollHeight });
      inputEl.value = "";
    }
  };
  inputEl.addEventListener("keydown", keyDownListener);
};

const init = () => {
  const inputEl = document.getElementById("input");
  const outputEl = document.getElementById("output");
  if (
    !(inputEl instanceof HTMLTextAreaElement) ||
    !(outputEl instanceof HTMLDivElement)
  ) {
    console.error("Couldn't get dom elements");
    return;
  }
  inputEl.focus();
  addInputListener(inputEl, outputEl);
};

window.onload = init;
