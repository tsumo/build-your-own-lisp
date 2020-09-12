import { sequenceParsers } from "./parser-combinators";
import { parseOperation, parseNumber, parseEof } from "./parsers";
import { ParseResult } from "./types";

const reportResult = (result: ParseResult<any>): string => {
  if (result.kind === "failure") {
    return `PARSING FAILURE, expected '${result.expected}', actual '${result.actual}'`;
  } else {
    return JSON.stringify({ data: result.data, rest: result.rest });
  }
};

type ParsedData = [
  number,
  (num1: number, num2: number) => number,
  number,
  null
];

const handleParsed = <T extends ParsedData>(...args: T) => {
  const [num1, operationHandler, num2] = args;
  return operationHandler(num1, num2);
};

/** Parses expressions of shape "<int><op><int>" into a resulting number */
export const parse = (input: string) =>
  reportResult(
    sequenceParsers<ParsedData, number>(handleParsed, [
      parseNumber,
      parseOperation,
      parseNumber,
      parseEof,
    ])(input)
  );
