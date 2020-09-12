import { sequenceParsers } from "./parser-combinators";
import { parsePlus, parseInteger, parseEof } from "./parsers";

type ParsedData = [number, string, number, null];

const handleParsed = <T extends ParsedData>(...args: T) => {
  const [num1, _, num2] = args;
  return num1 + num2;
};

/** Parses expressions of shape "int+int" into a resulting number */
export const parse = sequenceParsers<ParsedData, number>(handleParsed, [
  parseInteger,
  parsePlus,
  parseInteger,
  parseEof,
]);
