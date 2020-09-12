import { ParseResultSuccess, ParseResultFailure } from "./types";

export const reportSuccess = (success: ParseResultSuccess<any>): string =>
  JSON.stringify({ data: success.data, rest: success.rest });

export const reportFailure = (failure: ParseResultFailure): string =>
  `PARSING FAILURE, expected '${failure.expected}', actual '${failure.actual}'`;
