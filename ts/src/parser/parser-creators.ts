import { success, failure } from './parse-result-creators'
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
