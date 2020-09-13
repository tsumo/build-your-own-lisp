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

const oneOfOperations = oneOfParsers(
  createTextParser('+'),
  createTextParser('-'),
  createTextParser('*'),
  createTextParser('/'),
  createTextParser('^'),
)

const operationHandlers = {
  '+': (...nums: number[]) => nums.reduce((prev, curr) => prev + curr),
  '-': (...nums: number[]) => nums.reduce((prev, curr) => prev - curr),
  '*': (...nums: number[]) => nums.reduce((prev, curr) => prev * curr),
  '/': (...nums: number[]) => nums.reduce((prev, curr) => prev / curr),
  '^': (...nums: number[]) => nums.reduce((prev, curr) => prev ** curr),
}

export const parseOperation = (input: string) =>
  mapParserResult((op) => operationHandlers[op], oneOfOperations)(input)

export const parseEof: Parser<null> = (input: string) => {
  if (input.length === 0) {
    return success(null, input)
  }
  return failure('end of input', input)
}
