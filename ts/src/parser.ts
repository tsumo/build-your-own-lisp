type ParseResultSuccess<T> = {
  kind: "success";
  data: T;
  rest: string;
};

type ParseResultFailure = {
  kind: "failure";
  expected: string;
  actual: string;
};

type ParseResult<T> = ParseResultSuccess<T> | ParseResultFailure;

const success = <T>(data: T, rest: string): ParseResultSuccess<T> => ({
  kind: "success",
  data,
  rest,
});

const failure = (expected: string, actual: string): ParseResultFailure => ({
  kind: "failure",
  expected,
  actual,
});

type Parser<T> = (input: string) => ParseResult<T>;

const parseInterger: Parser<number> = (input) => {
  const match = /^\d+/.exec(input);
  if (match === null) {
    return failure("integer", input);
  }
  const matchedText = match[0];
  return success(+matchedText, input.slice(matchedText.length));
};

const parsePlus: Parser<string> = (input) => {
  if (input[0] === "+") {
    return success("+", input.slice(1));
  }
  return failure("+", input);
};

const reportSuccess = (success: ParseResultSuccess<any>): string =>
  JSON.stringify({ data: success.data, rest: success.rest });

const reportFailure = (failure: ParseResultFailure): string =>
  `PARSING FAILURE, expected '${failure.expected}', actual '${failure.actual}'`;

export const parse = (input: string): string => {
  const result1 = parseInterger(input);
  if (result1.kind === "failure") {
    return reportFailure(result1);
  }
  const { data: int1, rest: input1 } = result1;

  const result2 = parsePlus(input1);
  if (result2.kind === "failure") {
    return reportFailure(result2);
  }
  const { rest: input2 } = result2;

  const result3 = parseInterger(input2);
  if (result3.kind === "failure") {
    return reportFailure(result3);
  }
  const { data: int2, rest: input3 } = result3;

  if (input3.length > 0) {
    return reportFailure(failure("end of input", input3));
  }

  return reportSuccess(success(int1 + int2, input3));
};
