import { oneOfParsers, sequenceParsers } from './../parser/parser-combinators'
import { createTextParser, labelParser } from './../parser/parser-creators'
import { parseEof, parseNumber, parseOperation, parseSpaces, parseToken } from './../parser/parsers'
import { ParseResult } from './../parser/types'

const reportResult = (result: ParseResult<any>): string => {
  if (result.kind === 'failure') {
    return `PARSING FAILURE, expected '${result.expected}', actual '${result.actual}'`
  } else {
    return String(result.data)
  }
}

const parseOpenParen = createTextParser('(')

const parseCloseParen = createTextParser(')')

const parseApplication = sequenceParsers<
  [string, string, (num1: number, num2: number) => number, number, number, string, null],
  number
>((spaces, open, op, num1, num2) => op(num1, num2), [
  parseSpaces,
  parseToken(parseOpenParen),
  parseToken(labelParser(parseOperation, 'an arithmetic operation')),
  parseToken(parseNumber),
  parseToken(parseNumber),
  parseToken(parseCloseParen),
  parseEof,
])

const parseExpression = labelParser(
  oneOfParsers(parseNumber, parseApplication),
  'number or application',
)

export const parse = (input: string) => reportResult(parseExpression(input))
