import { ParseResultSuccess, ParseResultFailure } from './types'

export const success = <T>(data: T, rest: string): ParseResultSuccess<T> => ({
  kind: 'success',
  data,
  rest,
})

export const failure = (expected: string, actual: string): ParseResultFailure => ({
  kind: 'failure',
  expected,
  actual,
})
