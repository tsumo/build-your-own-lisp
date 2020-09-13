import { success, failure } from './parse-result-creators'
import { Parser, ParseResult } from './types'

/** Generalized tuple */
type Arr = readonly unknown[]

/** Mapping tuple type to tuple of parsers */
type Parsers<T extends Arr> = {
  [K in keyof T]: Parser<T[K]>
}

/** Handler accepting variadic tuple type */
type ResultHandler<T extends Arr, R> = (...args: [...T]) => R

export const sequenceParsers = <T extends Arr, R>(
  handleParsed: ResultHandler<T, R>,
  parsers: Parsers<T>,
) => (input: string): ParseResult<R> => {
  // I don't know how to express partial variadic tuple
  // Probably have to write custom "push" function
  const parsedData: [...T] = ([] as unknown) as [...T]
  let currentInput = input

  for (const parser of parsers) {
    const result = parser(currentInput)
    if (result.kind === 'failure') {
      return result
    }
    parsedData.push(result.data)
    currentInput = result.rest
  }

  return success(handleParsed(...parsedData), currentInput)
}

/** Uses indexed access type */
export const oneOfParsers = <T extends Arr>(...parsers: [...Parsers<T>]) => (
  input: string,
): ParseResult<T[number]> => {
  for (const parser of parsers) {
    const result = parser(input)
    if (result.kind === 'failure') {
      continue
    }
    // Again, not sure how to enforce type safety here
    return result as ParseResult<T[number]>
  }
  return failure('oneOf', input)
}
