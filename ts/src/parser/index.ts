import { sequenceParsers } from "./parser-combinators";
import { parsePlus, parseInteger, parseEof } from "./parsers";
import { ParseResult } from "./types";

const reportResult = (result: ParseResult<any>): string => {
  if (result.kind === "failure") {
    return `PARSING FAILURE, expected '${result.expected}', actual '${result.actual}'`;
  } else {
    return JSON.stringify({ data: result.data, rest: result.rest });
  }
};

type ParsedData = [number, string, number, null];

const handleParsed = <T extends ParsedData>(...args: T) => {
  const [num1, _, num2] = args;
  return num1 + num2;
};

/** Parses expressions of shape "int+int" into a resulting number */
export const parse = (input: string) =>
  reportResult(
    sequenceParsers<ParsedData, number>(handleParsed, [
      parseInteger,
      parsePlus,
      parseInteger,
      parseEof,
    ])(input)
  );
