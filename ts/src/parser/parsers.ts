import { success, failure } from './parse-result-creators'
import { oneOfParsers } from './parser-combinators'
import {
  createLexemeParser,
  createRegexParser,
  createTextParser,
  mapParserResult,
} from './parser-creators'
import { Parser } from './types'

export const parseSpaces = createRegexParser(/\s*/)

export const parseToken = createLexemeParser(parseSpaces)

export const parseNumber = mapParserResult((x) => +x, createRegexParser(/\d+(?:\.\d+)?/))

const binaryOperationHandlers = {
  '+': (num1: number, num2: number) => num1 + num2,
  '-': (num1: number, num2: number) => num1 - num2,
  '*': (num1: number, num2: number) => num1 * num2,
  '/': (num1: number, num2: number) => num1 / num2,
  '^': (num1: number, num2: number) => num1 ** num2,
}

export const parseBinaryOperation = (input: string) => {
  const oneOf = oneOfParsers(
    createTextParser('+'),
    createTextParser('-'),
    createTextParser('*'),
    createTextParser('/'),
    createTextParser('^'),
  )
  return mapParserResult((op) => binaryOperationHandlers[op], oneOf)(input)
}

export const parseEof: Parser<null> = (input: string) => {
  if (input.length === 0) {
    return success(null, input)
  }
  return failure('end of input', input)
}
