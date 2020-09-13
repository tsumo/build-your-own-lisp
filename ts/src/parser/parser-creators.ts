import { success, failure } from './parse-result-creators'
import { sequenceParsers } from './parser-combinators'
import { Parser } from './types'

export const mapParserResult = <T, R>(map: (result: T) => R, parser: Parser<T>) => (
  input: string,
) => {
  const result = parser(input)
  if (result.kind === 'failure') {
    return result
  }
  return success(map(result.data), result.rest)
}

export const labelParser = <T>(parser: Parser<T>, expected: string) => (input: string) => {
  const result = parser(input)
  if (result.kind === 'failure') {
    return failure(expected, result.actual)
  }
  return result
}

export const createTextParser = <T extends string>(match: T): Parser<T> => (input: string) => {
  if (input.startsWith(match)) {
    return success(match, input.slice(match.length))
  }
  return failure(match, input)
}

export const createRegexParser = (regex: RegExp): Parser<string> => {
  const anchoredRegex = new RegExp(`^${regex.source}`)
  return (input: string) => {
    const match = anchoredRegex.exec(input)
    if (match === null) {
      return failure(`Regex ${anchoredRegex.source}`, input)
    }
    const matchedText = match[0]
    return success(matchedText, input.slice(matchedText.length))
  }
}

/** Lexeme parser parses meaningful data and then skips the junk */
export const createLexemeParser = <T>(junkParser: Parser<T>) => <U>(parser: Parser<U>) => {
  const handleParsed = (data: U, _: T) => data
  return sequenceParsers(handleParsed, [parser, junkParser])
}
