import { parse } from './parser/index'

const addInputListener = (inputEl: HTMLTextAreaElement, outputEl: HTMLDivElement) => {
  let lastCommand = ''

  const keyDownListener = (e: KeyboardEvent) => {
    if (e.code === 'ArrowUp' && inputEl.selectionStart === 0) {
      e.preventDefault()
      inputEl.value = lastCommand
      return
    }

    if (e.code === 'Enter' && e.ctrlKey) {
      e.preventDefault()
      inputEl.value += '\n'
      return
    }

    if (e.code === 'Enter') {
      e.preventDefault()
      if (inputEl.value.trim().length === 0) {
        return
      }
      const trimmedValue = inputEl.value.trim()
      outputEl.innerHTML += `<p class="line">${parse(trimmedValue)}<p />`
      outputEl.scrollTo({ top: outputEl.scrollHeight })
      lastCommand = trimmedValue
      inputEl.value = ''
      return
    }
  }
  inputEl.addEventListener('keydown', keyDownListener)
}

const init = () => {
  const inputEl = document.getElementById('input')
  const outputEl = document.getElementById('output')
  if (!(inputEl instanceof HTMLTextAreaElement) || !(outputEl instanceof HTMLDivElement)) {
    console.error("Couldn't get dom elements")
    return
  }
  inputEl.focus()
  addInputListener(inputEl, outputEl)
}

window.onload = init
