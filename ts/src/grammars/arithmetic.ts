import { sequenceParsers } from './../parser/parser-combinators'
import { labelParser } from './../parser/parser-creators'
import {
  parseBinaryOperation,
  parseNumber,
  parseEof,
  parseSpaces,
  parseToken,
} from './../parser/parsers'
import { ParseResult } from './../parser/types'

const reportResult = (result: ParseResult<any>): string => {
  if (result.kind === 'failure') {
    return `PARSING FAILURE, expected '${result.expected}', actual '${result.actual}'`
  } else {
    return JSON.stringify({ data: result.data, rest: result.rest })
  }
}

type ParsedData = [string, number, (num1: number, num2: number) => number, number, null]

const handleParsed = <T extends ParsedData>(...args: T) => {
  const [_, num1, operationHandler, num2] = args
  return operationHandler(num1, num2)
}

/** Parses expressions of shape "<int><op><int>" into a resulting number */
export const parse = (input: string) =>
  reportResult(
    sequenceParsers<ParsedData, number>(handleParsed, [
      parseSpaces, // skip leading spaces
      parseToken(parseNumber),
      parseToken(labelParser(parseBinaryOperation, 'an arithmetic operator')),
      parseToken(parseNumber), // will skip trailing spaces
      parseEof,
    ])(input),
  )
