export type ParseResultSuccess<T> = {
  kind: 'success'
  data: T
  rest: string
}

export type ParseResultFailure = {
  kind: 'failure'
  expected: string
  actual: string
}

export type ParseResult<T> = ParseResultSuccess<T> | ParseResultFailure

export type Parser<T> = (input: string) => ParseResult<T>
