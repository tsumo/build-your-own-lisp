import {
  createRegexParser,
  createTextParser,
  mapParserResult,
} from "./parser-creators";

export const parsePlus = createTextParser("+");

export const parseInteger = mapParserResult(
  (x) => +x,
  createRegexParser(/\d+/)
);
